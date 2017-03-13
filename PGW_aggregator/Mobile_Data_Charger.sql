CREATE OR REPLACE 
PACKAGE mobile_data_charger IS
	function ChargeSession(pRowid in rowid) return number;
	function ExportSession(
   		pchargingID in integer,
    	pimsi in integer,
        pmsisdn in integer,
        pimei in varchar2,
        paccessPointName in varchar2,
        pstartTime date,
        pendTime date,
        pservingNodeIP in integer,
        pplmnID in integer,
        pratingGroup in integer,
        pvolumeUplink in integer,
        pvolumeDownlink in integer
	) return number;
	function IsErrorStatus(pStatus in integer) return integer;
    procedure RegisterFileStats(pFilename in varchar2, pVolumeUplink in integer, pVolumeDownlink in integer,
		pRecordCount in integer, pEarliestTime in date, pLatestTime in date, pFileTimestamp in date, pProcessTimeSec in integer);
    procedure SendAlert(pMessage in varchar2);
    function ChargingAllowed return integer;
    procedure ChargePostponed(pProcIndex in integer, pProcCount in integer);

    procedure ClearMobileSessions;
    procedure CheckTestExport;
    procedure GetSubscrAttributesTest;
    procedure GetNetworkAttributesTest;
    procedure SetTariffAttributesTest;
	procedure GetChargedVolumeMbTest;
    procedure GetServiceRateTest;
    procedure ExportSessionTest;
    procedure RunAllTests;
END;
/

CREATE OR REPLACE 
PACKAGE BODY mobile_data_charger IS

MEGABYTE_SIZE_IN_BYTES 			constant integer := 1048576;
NUM_OF_SECONDS_IN_DAY			constant number := 60*60*24;
MEGABYTE_UNIT_ID				constant number := 111;
DEFAULT_NETWORK_CLASS			constant number := 272;
SESSION_LOOKUP_TIMEOUT			constant integer := 7;

SESSION_STATUS_READY_TO_CHARGE	constant integer := 0;
SESSION_STATUS_CHARGED			constant integer := 1;

TRAFFIC_TYPE_DOWNLINK		constant integer :=	287304;
TRAFFIC_TYPE_UPLINK			constant integer :=	287303;

TEST_IMSI 					constant integer := 250270700285472;  -- subscriber have tariff plan and rate change
SAMPLE_MIN_DATE				constant date := to_date('30.05.2016 20:51:19','dd.mm.yyyy hh24:mi:ss');
SAMPLE_MAX_DATE				constant date :=  to_date('15.09.2016 13:49:36','dd.mm.yyyy hh24:mi:ss');


type SubscriberAttributes is record (
    contractID integer,
    tariffPlanID integer,
    contractCommonID integer,
    accountID integer,
    providerID integer,
    clientID integer,
    clientTypeID integer,
    companyBranchID integer,
    scheduleID integer,
    calendarID integer
);

type NetworkAttributes is record (
    mobileNetworkID TMobileNetwork.object_no%type,
    mscID integer,
    homeCountry integer,
    homeNetwork integer,
    homeMSC integer,
    roamingCountryID integer,
    countryCode TRoamingCountry.country_code%type,
    pseudoRoamingHubID integer
);

type TariffAttributes is record (
    accessTypeID integer,
    accessZoneID integer,
    dayClassID integer,
    timeClassID integer,
    localityID integer,
    roamingZoneID integer,
  	uplinkRatePerMb number,
  	downlinkRatePerMb number
);

SubscriberNotFound exception;
pragma exception_init(SubscriberNotFound, -20001);
MultipleSubscribersFound exception;
pragma exception_init(MultipleSubscribersFound, -20002);
NetworkNotFound exception;
pragma exception_init(NetworkNotFound, -20003);
MultipleNetworksFound exception;
pragma exception_init(MultipleNetworksFound, -20004);
ApnAnalyzeError exception;
pragma exception_init(ApnAnalyzeError , -20005);
BadTrafficType exception;
pragma exception_init(BadTrafficType , -20006);
ClassificationError exception;
pragma exception_init(ClassificationError, -20007);
GetRateError exception;
pragma exception_init(GetRateError, -20008);

function ChargingAllowed return integer is
	vAllowed integer;
begin
	return IRBiS.GetSystemParamValueN('Telephony charging allowed', sysdate);
end;


function AccountBalanceIsLocked(pAccountID in integer) return boolean is
	vCurrentBalance number;
begin
    select current_balance into vCurrentBalance from TAccount where object_no = pAccountID
        for update of current_balance nowait;
    return false;
exception
    when others then
    	return true;
end;


procedure DebugLog(pMessage in varchar2) is
pragma autonomous_transaction;
begin
	insert into MobileDataCharger_Log (datetime, message) values (sysdate, pMessage);
	commit;
end;


function IsErrorStatus(pStatus in integer)  return integer is
begin
	if pStatus <0 then return 1;
    else return 0;
    end if;
end;


function GetEndTime(pStartTime date, pDurationSeconds in integer) return date is
begin
	return pStartTime + pDurationSeconds/NUM_OF_SECONDS_IN_DAY;
end;

function GetNetworkByServingIP(pServingIP in integer, pDate in date) return NetworkAttributes is
    vAttrs NetworkAttributes;
begin
	select mnw.object_no, msc.object_no, nvl(rc.home_country,0), nvl(mnw.home_network,0), nvl(msc.home_msc,0),
            mnw.roamingcountry_id, rc.country_code, null
        into vAttrs
        from TMsc msc, TMobilenetwork mnw, TRoamingCountry rc
        where msc.start_ip <= pServingIP and pServingIP <= msc.end_ip
        and msc.date_in <= pDate and msc.date_out > pDate and mnw.object_no = msc.mobilenetwork_id
        and rc.object_no = mnw.roamingcountry_id;
	return vAttrs;
exception
	when no_data_found then
		raise NetworkNotFound;
	when too_many_rows then
		raise MultipleNetworksFound;
end;


function GetNetworkByPlmnID(pPlmnID in integer, pDate in date) return NetworkAttributes is
	vAttrs NetworkAttributes;
begin
	select mnw.object_no, 0 /*mscID*/, nvl(rc.home_country,0), nvl(mnw.home_network,0), 0 /*home_msc*/,
            mnw.roamingcountry_id, rc.country_code, null
        into vAttrs
        from TMobileNetwork mnw, TRoamingCountry rc
        where mnw.plmn_identifier = pPlmnID and rc.object_no = mnw.roamingcountry_id;
    return vAttrs;
exception
	when no_data_found then
		raise NetworkNotFound;
   when too_many_rows then
		raise MultipleNetworksFound;
end;


function GetNetworkAttributes(pPlmnID in integer, pServingIP in integer, pDate in date) return NetworkAttributes is
	COMFONE_ROAMING_HUB_ID			constant integer := 624467901;
	SPARKLE_ROAMING_HUB_ID			constant integer := 935076610;
	TELECOM_ITALIA_PLMN_ID			constant varchar2(5) := '22201';
	TELECOM_ITALIA_NETWORK_ID		constant integer := 567205114;
	vNetworkByServingIP NetworkAttributes;
    vNetworkByPlmnID NetworkAttributes;
    vResultedAttrs NetworkAttributes;
    vNetworkByServingIpFound boolean;
    vNetworkByPlmnIdFound boolean;
    vPseudoRoamingHubID integer;
begin
	begin
		vNetworkByServingIP := GetNetworkByServingIP(pServingIP, pDate);
        vNetworkByServingIpFound := true;
    exception
    when NetworkNotFound then
    	vNetworkByServingIpFound := false;
    end;

    begin
    	vNetworkByPlmnID := GetNetworkByPlmnID(pPlmnID, pDate);
        vNetworkByPlmnIdFound := true;
    exception
    when NetworkNotFound then
    	vNetworkByPlmnIdFound := false;
    end;

    /* доработка по запросу MOBILE-713 */
    vPseudoRoamingHubID := COMFONE_ROAMING_HUB_ID;
    if vNetworkByServingIP.mobileNetworkID = TELECOM_ITALIA_NETWORK_ID then
    	if vNetworkByPlmnIdFound then
   	    	vResultedAttrs := vNetworkByPlmnID;
        	if pPlmnID != TELECOM_ITALIA_PLMN_ID then
                vPseudoRoamingHubID := SPARKLE_ROAMING_HUB_ID;
            end if;
        else
        	raise NetworkNotFound;
        end if;
    else
    	if vNetworkByServingIpFound then
    		vResultedAttrs := vNetworkByServingIP;
        elsif vNetworkByPlmnIdFound then
        	vResultedAttrs := vNetworkByPlmnID;
        else
        	raise NetworkNotFound;
        end if;
    end if;

    vResultedAttrs.pseudoRoamingHubID := vPseudoRoamingHubID;
    return vResultedAttrs;
end;


function GetSubscriberAttributes(pImsi in integer, pStartTime in date) return SubscriberAttributes is
	vAttrs SubscriberAttributes;
begin
	select c.object_no contract_id, c.tariffplan_id, c.contractcommon_id, a.object_no, a.provider_id,
            a.client_id, cl.clienttype_id, a.companybranch_id, p.schedule_id, p.calendar_id
            into vAttrs
        from TSimCard sim, TContractMobile cm, TContractCommon cc, TContract c, TAccount a, TClient cl, TTariffPlan p
        where sim.sim_imsi = to_char(pImsi) and cm.simcard_id = sim.object_no
        and cm.object_no = c.object_no and c.date_in <= pStartTime and c.date_out > pStartTime
        and p.object_no = c.tariffplan_id
        and cc.object_no = c.contractcommon_id and cc.contract_date <= pStartTime and cc.contract_end > pStartTime
        and pStartTime < nvl(cc.stop_charge, pStartTime +1)
        and a.object_no = c.account_id and cl.object_no = a.client_id;
    return vAttrs;
exception
	when no_data_found then
		raise SubscriberNotFound;
	when too_many_rows then
		raise MultipleSubscribersFound;
end;


function GetTariffAttributes(pApn in varchar2, pNetworkAttrs in out nocopy NetworkAttributes, pStartTime in date)
		return TariffAttributes is
	GPRS_ANALYZE_BRANCH			constant integer := 226792045;
    KAZAN_NUMBER_SITING         constant integer := 884755;
    INTRANETWORK_ROAMING		constant integer := 228020139;
    NATIONAL_ROAMING			constant integer := 228020140;
    INTERNATIONAL_ROAMING		constant integer := 228020141;
	vAttrs TariffAttributes;
    vAnalyzeRes integer;
    vOutNumber varchar2(100);
begin
	if pNetworkAttrs.homeMsc > 0 then
    	vAnalyzeRes := PTelephony.AnalyzeNumber(GPRS_ANALYZE_BRANCH, pApn, KAZAN_NUMBER_SITING, 0, pStartTime,
            vAttrs.accessTypeID, vAttrs.accessZoneID, vAttrs.localityID, vOutNumber);
        if vAnalyzeRes < 0 then
            raise ApnAnalyzeError;
        end if;
    else
    	if pNetworkAttrs.homeNetwork > 0 then
            vAttrs.accessTypeID := INTRANETWORK_ROAMING;
        elsif pNetworkAttrs.homeCountry > 0 then
            vAttrs.accessTypeID := NATIONAL_ROAMING;
        else
            vAttrs.accessTypeID := INTERNATIONAL_ROAMING;
        end if;
        if pNetworkAttrs.mscID > 0 then
            vAttrs.roamingZoneID := pNetworkAttrs.mscID;
        else
            vAttrs.roamingZoneID := pNetworkAttrs.mobileNetworkID;
        end if;
    end if;
    return vAttrs;
end;


function ClassifyService(pTrafficType in integer, pRatingGroup in integer, pStartDate in date, pTariffAttrs in out nocopy TariffAttributes,
		pSubscriberAttrs in out nocopy SubscriberAttributes) return integer is
    SCALE_CLASSIFICATOR         constant integer := 20;
	pTrafficTypeID integer;
    vServiceID integer;
begin
	begin
		select object_no into pTrafficTypeID from TTelephonyType
			where traffictype_id = pTrafficType and pcrf_rating_group = pRatingGroup;
    exception when no_data_found then
    	raise BadTrafficType;
    end;
	vServiceID := PScales.GetValue(SCALE_CLASSIFICATOR, pTrafficTypeID,
    	pTariffAttrs.accessTypeID, pTariffAttrs.accessZoneID, pSubscriberAttrs.providerID, 0 /*privileged phone type*/,
        pSubscriberAttrs.tariffPlanID, 0, 0, 0, 0, 0, 0, pStartDate);
	if vServiceID < 0 then
		raise ClassificationError;
	end if;
	return vServiceID;
end;


function GetChargedVolumeMB(pServiceID in integer, pSubscriberAttrs in out nocopy SubscriberAttributes,
		pVolumeBytes in integer, pDate date) return number is
	SCALE_CHARGINGUNITS    constant integer := 30;
	vUnits number;
	vChargedVolume number;
begin
	if pVolumeBytes = 0 then
    	return 0;
    end if;
	vUnits := PScales.GetValue(SCALE_CHARGINGUNITS, pServiceID, pSubscriberAttrs.clientTypeID, pSubscriberAttrs.tariffPlanID,
    	0, 0, 0, 0, 0, 0, 0, pVolumeBytes, 0, pDate);
	if vUnits >= 0 then
		--- volume rounding set
		if vUnits = 0 then
			vChargedVolume := 0;
		else
        	-- apply rounding
			if abs(pVolumeBytes/vUnits - trunc(pVolumeBytes/vUnits)) < 1/MEGABYTE_SIZE_IN_BYTES then
				-- no rounding is needed
				vChargedVolume := pVolumeBytes / MEGABYTE_SIZE_IN_BYTES;
			else
				vChargedVolume := vUnits * trunc(pVolumeBytes/vUnits + 1) / MEGABYTE_SIZE_IN_BYTES;
			end if;
		end if;
	else
    	-- volume rounding is not set
		vChargedVolume := pVolumeBytes/MEGABYTE_SIZE_IN_BYTES;
	end if;
	return vChargedVolume;
end;


function GetServiceRate(pServiceID in integer, pStartTime date,
        pSubscriberAttrs in out nocopy SubscriberAttributes,
        pNetworkAttrs in out nocopy NetworkAttributes,
        pTariffAttrs in out nocopy TariffAttributes) return number is
    vRate number;
begin
    if pNetworkAttrs.homeMSC > 0 then
    	vRate := PCharging.GetTelephonyCallCostIC(pServiceID, pSubscriberAttrs.contractID, pTariffAttrs.accesszoneID,
        	pTariffAttrs.dayClassID, pTariffAttrs.timeClassID, pStartTime);
    else
    	vRate := PCharging.GetRoamingCallCostIC(pSubscriberAttrs.contractID, pServiceID,
        	pNetworkAttrs.roamingCountryID, pNetworkAttrs.mobileNetworkID,
        	null /*pAccessZoneID*/, pNetworkAttrs.pseudoRoamingHubID, pStartTime);
    end if;
    if vRate < 0 then
    	raise GetRateError;
    end if;
    return vRate;
end;


function GetServiceIdAndRate(pTrafficType in integer, pRatingGroup in integer, pStartTime in date,
        pSubscriberAttrs in out nocopy SubscriberAttributes,
        pNetworkAttrs in out nocopy NetworkAttributes,
        pTariffAttrs in out nocopy TariffAttributes,
        pRatePerMb out number) return number is
	vServiceID integer;
begin
	vServiceID := ClassifyService(pTrafficType, pRatingGroup, pStartTime, pTariffAttrs, pSubscriberAttrs);
    pRatePerMb := GetServiceRate(vServiceID, pStartTime, pSubscriberAttrs, pNetworkAttrs, pTariffAttrs);
    return vServiceID;
end;


procedure ClassifyAndGetRates(pservingNodeIP in integer, pplmnID in integer, pratingGroup in integer,
		paccessPointName in varchar2, pSubscrAttrs in out nocopy SubscriberAttributes,
		pStartTime in date, pUplinkServiceID out integer, pDownlinkServiceID out integer,
        pUplinkRatePerMb out number, pDownlinkRatePerMb out number, pRoamingZoneId out number) is
	vNetworkAttrs NetworkAttributes;
    vTariffAttrs TariffAttributes;
begin
	vNetworkAttrs := GetNetworkAttributes(pplmnID, pservingNodeIP, pStartTime);
    vTariffAttrs := GetTariffAttributes(paccessPointName, vNetworkAttrs, pStartTime);
    pUplinkServiceID := GetServiceIdAndRate(TRAFFIC_TYPE_UPLINK, pratingGroup, pStartTime,
        pSubscrAttrs, vNetworkAttrs, vTariffAttrs, pUplinkRatePerMb);
    pDownlinkServiceID := GetServiceIdAndRate(TRAFFIC_TYPE_DOWNLINK, pRatingGroup, pStartTime,
        pSubscrAttrs, vNetworkAttrs, vTariffAttrs, pDownlinkRatePerMb);
    if vNetworkAttrs.homeMSC = 0 then
        if vNetworkAttrs.mscID > 0 then
            pRoamingZoneId := vNetworkAttrs.mscID;
        else
            pRoamingZoneId := vNetworkAttrs.mobileNetworkID;
        end if;
    else
        pRoamingZoneId := null;
    end if;
end;


-- Function return value: max of uplink and downlink rates
function ChargeSession(pRowid in rowid) return number is
	TRAFFIC_POSSIBLY_RATED constant number := 1;
	vSession Mobile_Session%rowtype;
    vSubscrAttrs SubscriberAttributes;
    vTariffAttrs TariffAttributes;
    vUplinkServiceID integer;
    vDownlinkServiceID integer;
    vChargedVolumeUplinkMb number;
    vChargedVolumeDownlinkMb number;
    vUplinkRatePerMb number := 0;
    vDownlinkRatePerMb number := 0;
    vOriginID integer;
    vUplinkDetailID integer;
    vDownlinkDetailID integer;
    vRoamingZoneID integer;
    vErrorInfoMaxLength constant integer := 1000;
    vErrorInfo varchar2(1000);
begin
	if ChargingAllowed = 0 then
    	return 0;
    end if;
    begin
        select * into vSession
            from Mobile_Session
            where rowid = pRowid and status = SESSION_STATUS_READY_TO_CHARGE
            for update;
    exception when no_data_found then
    	return 0;
    end;

    if vSession.uplink_postponed = 0 and vSession.downlink_postponed = 0 then
    	update Mobile_Session set status = SESSION_STATUS_CHARGED where rowid = pRowid;
        return 0;
    end if;

    begin
    vSubscrAttrs := GetSubscriberAttributes(vSession.imsi, vSession.start_time);

    if AccountBalanceIsLocked(vSubscrAttrs.accountID) then
    	return TRAFFIC_POSSIBLY_RATED;
    end if;

    vChargedVolumeUplinkMb := vSession.uplink_postponed / MEGABYTE_SIZE_IN_BYTES;
    vChargedVolumeDownlinkMb := vSession.downlink_postponed / MEGABYTE_SIZE_IN_BYTES;
    if vSession.origin_id is null then
        ClassifyAndGetRates(vSession.serving_node_ip, vSession.plmn_id, vSession.rating_group,
            vSession.access_point_name, vSubscrAttrs,
            vSession.start_time, vUplinkServiceID, vDownlinkServiceID,
            vUplinkRatePerMb, vDownlinkRatePerMb, vRoamingZoneID);
        select Origin_Seq.nextval into vOriginID from dual;
        vUplinkDetailID := PCharging.ChargeInetService(vSubscrAttrs.contractID, vSession.access_point_name, vUplinkServiceID,
            MEGABYTE_UNIT_ID, DEFAULT_NETWORK_CLASS, vSession.start_time, GetEndTime(vSession.start_time, vSession.duration),
            vChargedVolumeUplinkMb, vUplinkRatePerMb, vOriginID, vRoamingZoneID);
        vDownlinkDetailID := PCharging.ChargeInetService(vSubscrAttrs.contractID, vSession.access_point_name, vDownlinkServiceID,
            MEGABYTE_UNIT_ID, DEFAULT_NETWORK_CLASS, vSession.start_time, GetEndTime(vSession.start_time, vSession.duration),
            vChargedVolumeDownlinkMb, vDownlinkRatePerMb, vOriginID, vRoamingZoneID);
        update Mobile_Session set
            uplink_rate = vUplinkRatePerMb,
            downlink_rate = vDownlinkRatePerMb,
            origin_id = vOriginID,
            uplink_detail_id = vUplinkDetailID,
            downlink_detail_id = vDownlinkDetailID
        where rowid = pRowid;
    else
        PCharging.UpdateInetChargeIfPossible(vSession.origin_id, vSession.uplink_detail_id,
            vSubscrAttrs.contractID, vSession.start_time, GetEndTime(vSession.start_time, vSession.duration), SESSION_LOOKUP_TIMEOUT,
            vChargedVolumeUplinkMb, vSession.uplink_rate);
        PCharging.UpdateInetChargeIfPossible(vSession.origin_id, vSession.downlink_detail_id,
            vSubscrAttrs.contractID, vSession.start_time, GetEndTime(vSession.start_time, vSession.duration), SESSION_LOOKUP_TIMEOUT,
            vChargedVolumeDownlinkMb, vSession.downlink_rate);
    end if;
    update Mobile_Session set
        status = SESSION_STATUS_CHARGED,
        uplink_charged = uplink_charged + uplink_postponed,
        downlink_charged = downlink_charged + downlink_postponed,
        uplink_postponed = 0,
        downlink_postponed = 0,
        errorinfo = null
    where rowid = pRowid;

	exception
	when SubscriberNotFound then
   		update Mobile_Session set status = PMobile.ERROR_CONTRACT_NOTFOUND where rowid = pRowid;
	when MultipleSubscribersFound then
		update Mobile_Session set status = PMobile.ERROR_DUPLICATECONTRACT where rowid = pRowid;
	when NetworkNotFound then
		update Mobile_Session set status = PMobile.ERROR_NETWORK_UNKNOWN where rowid = pRowid;
	when MultipleNetworksFound then
		update Mobile_Session set status = PMobile.ERROR_DUPLICATE_NETWORK where rowid = pRowid;
	when ApnAnalyzeError then
		update Mobile_Session set status = PMobile.ERROR_NO_PREFIX where rowid = pRowid;
	when BadTrafficType then
		update Mobile_Session set status = PMobile.BAD_TRAFFIC_TYPE where rowid = pRowid;
	when ClassificationError then
		update Mobile_Session set status = PMobile.ERROR_CLASSIFICATION where rowid = pRowid;
	when GetRateError then
		update Mobile_Session set status = PMobile.ERROR_CALC_PRICE where rowid = pRowid;
    when others then
    	vErrorInfo := substr(SQLErrM || utl_tcp.CRLF || dbms_utility.format_error_backtrace, 1, vErrorInfoMaxLength);
    	update Mobile_Session set status = PMobile.ERROR_UNKNOWN, errorinfo = vErrorInfo where rowid = pRowid;
    end;
    return greatest(vUplinkRatePerMb, vDownlinkRatePerMb);
end;


function CreateNewSession(
            pchargingID in integer,
            pimsi in integer,
            pmsisdn in integer,
            pimei in varchar2,
            paccessPointName in varchar2,
            pstartTime in date,
            pendTime in date,
            pservingNodeIP in integer,
            pplmnID in integer,
            pratingGroup in integer,
            pvolumeUplink in integer,
            pvolumeDownlink in integer
        ) return rowid is
    vSessionRowid rowid;
begin
	insert into Mobile_Session (
                charging_id,
                imsi,
                msisdn,
                imei,
                access_point_name,
                start_time,
                duration,
                serving_node_ip,
                plmn_id,
                rating_group,
                created,
                update_count,
                last_updated,
                closed,
                uplink_charged,
                downlink_charged,
                uplink_postponed,
                downlink_postponed,
                status
                )
            values (
                pchargingID,
                pimsi,
                pmsisdn,
                pimei,
                paccessPointName,
                pstartTime,
                (pendTime - pstartTime) * NUM_OF_SECONDS_IN_DAY,
                pservingNodeIP,
                pplmnID,
                pratingGroup,
                sysdate,
                0, -- update_count
                sysdate,
                null, --closed,
                0, -- uplink_charged
                0, -- downlink_charged
                pvolumeUplink, -- uplink_postponed
                pvolumeDownlink, -- downlink_postponed
                SESSION_STATUS_READY_TO_CHARGE
            ) returning rowid into vSessionRowid;
    return vSessionRowid;
end;


procedure CloseSession(pRowid in rowid) is
begin
	update Mobile_Session set closed = sysdate where rowid = pRowid;
end;




procedure UpdateSessionAttributes(pSessionRowid in rowid, pNewStartTime in date,
		pNewEndTime in date, pUpdatedStartTime out date, pUpdatedEndTime out date) is
	vSession Mobile_Session%rowtype;
begin
	select * into vSession from MOBILE_SESSION where rowid = pSessionRowid;
	pUpdatedStartTime := least(vSession.start_time, pNewStartTime);
    pUpdatedEndTime := greatest(GetEndTime(vSession.start_time, vSession.duration), pNewEndTime);
    update Mobile_Session set
            start_time = pUpdatedStartTime,
            duration = (pUpdatedEndTime - pUpdatedStartTime) * NUM_OF_SECONDS_IN_DAY,
            update_count = update_count + 1,
            last_updated = sysdate
            where rowid = pSessionRowid;
end;


function ExportSession(
   		pchargingID in integer,
    	pimsi in integer,
        pmsisdn in integer,
        pimei in varchar2,
        paccessPointName in varchar2,
        pstartTime in date,
        pendTime in date,
        pservingNodeIP in integer,
        pplmnID in integer,
        pratingGroup in integer,
        pvolumeUplink in integer,
        pvolumeDownlink in integer
	) return number is
    vSessionCount integer;
    vSessionRowid rowid;
    vSubscrAttrs SubscriberAttributes;
    vUplinkServiceID integer;
    vDownlinkServiceID integer;
    vRoamingZoneId integer;
    vUplinkRatePerMb number;
    vDownlinkRatePerMb number;
    vRatePerMb number;
    vUpdatedStartTime date;
    vUpdatedEndTime date;
    vCanBeUpdated boolean;
    vNeedToClose boolean;
    vNeedToPostponeCharging boolean;
    cursor cGetChargedSessionWithSameRate is select rowid session_rowid, m.* from MOBILE_SESSION m
        where charging_id = pchargingID and rating_group = pratingGroup and imsi = pimsi and closed is null
        and start_time >= pstartTime - SESSION_LOOKUP_TIMEOUT
        and status = SESSION_STATUS_CHARGED
        and uplink_rate = vUplinkRatePerMb and downlink_rate = vDownlinkRatePerMb
        and trunc(start_time, 'mm') = trunc(pstartTime, 'mm');
	vSession cGetChargedSessionWithSameRate%rowtype;
    cursor cGetNotChargedSession is select rowid from MOBILE_SESSION
        where charging_id = pchargingID and rating_group = pratingGroup and imsi = pimsi and closed is null
        and start_time >= pstartTime - SESSION_LOOKUP_TIMEOUT
        and (status = SESSION_STATUS_READY_TO_CHARGE or IsErrorStatus(status) = 1)
        and trunc(start_time, 'mm') = trunc(pstartTime, 'mm');
begin
	-- set lock on MOBILE_SESSION records
	update MOBILE_SESSION m set
    	uplink_postponed = uplink_postponed + 0,
        downlink_postponed = downlink_postponed + 0,
        uplink_charged = uplink_charged + 0,
        downlink_charged = downlink_charged + 0,
        uplink_rate = uplink_rate + 0,
        downlink_rate = downlink_rate + 0,
        status = status + 0
    	where charging_id = pchargingID and rating_group = pratingGroup and imsi = pimsi and closed is null
        	and start_time >= pstartTime - SESSION_LOOKUP_TIMEOUT;
    vSessionCount := SQL%Rowcount;

	if vSessionCount = 0 then
    	vSessionRowid := CreateNewSession(pchargingID, pimsi, pmsisdn, pimei, paccessPointName,
        	pstartTime, pendTime, pservingNodeIP, pplmnID, pratingGroup, pvolumeUplink, pvolumeDownlink);
        return ChargeSession(vSessionRowid);
    end if;

	savepoint svp1;
	begin
        vSubscrAttrs := GetSubscriberAttributes(pImsi, pStartTime);
        ClassifyAndGetRates(pservingNodeIP, pplmnID, pratingGroup, paccessPointName, vSubscrAttrs, pStartTime,
            vUplinkServiceID, vDownlinkServiceID, vUplinkRatePerMb, vDownlinkRatePerMb, vRoamingZoneId);
        vCanBeUpdated := false;
        vNeedToClose := false;
        vNeedToPostponeCharging := false;
        open cGetChargedSessionWithSameRate;
        fetch cGetChargedSessionWithSameRate into vSession;
        if cGetChargedSessionWithSameRate%found then
            vCanBeUpdated := true;
        	vSessionRowid := vSession.session_rowid;
            UpdateSessionAttributes(vSessionRowid, pstartTime, pendTime, vUpdatedStartTime, vUpdatedEndTime);
            if ChargingAllowed > 0 and not AccountBalanceIsLocked(vSubscrAttrs.accountID) then
                -- try to charge new quant
                PCharging.UpdateInetChargeIfPossible(vSession.origin_id, vSession.uplink_detail_id,
                    vSubscrAttrs.contractID, vUpdatedStartTime, vUpdatedEndTime, SESSION_LOOKUP_TIMEOUT,
                    pvolumeUplink/MEGABYTE_SIZE_IN_BYTES, vUplinkRatePerMb);
                PCharging.UpdateInetChargeIfPossible(vSession.origin_id, vSession.downlink_detail_id,
                    vSubscrAttrs.contractID, vUpdatedStartTime, vUpdatedEndTime, SESSION_LOOKUP_TIMEOUT,
                    pvolumeDownlink/MEGABYTE_SIZE_IN_BYTES, vDownlinkRatePerMb);
            else
            	vNeedToPostponeCharging := true;
            end if;
        end if;
        close cGetChargedSessionWithSameRate;
    exception when PCharging.UpdateInetChargeImpossible then
    	rollback to svp1;
        vCanBeUpdated := false;
        vNeedToClose := true;
    when others then
        rollback to svp1;
        vCanBeUpdated := false;
    end;

    if vCanBeUpdated then
       if vNeedToPostponeCharging then
            update Mobile_Session set
                uplink_postponed = uplink_postponed + pvolumeUplink,
                downlink_postponed = downlink_postponed + pvolumeDownlink,
                status = SESSION_STATUS_READY_TO_CHARGE
                where rowid = vSessionRowid;
       else
            update Mobile_Session set
                uplink_charged = uplink_charged + pvolumeUplink,
                downlink_charged = downlink_charged + pvolumeDownlink
                where rowid = vSessionRowid;
       end if;
    else
        if vNeedToClose then
            CloseSession(vSessionRowid);
        end if;

        open cGetNotChargedSession;
        fetch cGetNotChargedSession into vSessionRowid;
        if cGetNotChargedSession%found then
			UpdateSessionAttributes(vSessionRowid, pstartTime, pendTime, vUpdatedStartTime, vUpdatedEndTime);
            update Mobile_Session set
                uplink_postponed = uplink_postponed + pvolumeUplink,
                downlink_postponed = downlink_postponed + pvolumeDownlink
                where rowid = vSessionRowid;
            vRatePerMb := ChargeSession(vSessionRowid);
        else
            vSessionRowid := CreateNewSession(pchargingID, pimsi, pmsisdn, pimei, paccessPointName,
                pstartTime, pendTime, pservingNodeIP, pplmnID, pratingGroup, pvolumeUplink, pvolumeDownlink);
            vRatePerMb := ChargeSession(vSessionRowid);
        end if;
        close cGetNotChargedSession;
    end if;
    return greatest(vUplinkRatePerMb, vDownlinkRatePerMb);
end;


procedure RegisterFileStats(pFilename in varchar2, pVolumeUplink in integer, pVolumeDownlink in integer,
	pRecordCount in integer, pEarliestTime in date, pLatestTime in date, pFileTimestamp in date, pProcessTimeSec in integer) is
begin
	insert into Pgw_Cdr_Stats (filename, uplink_total, downlink_total, record_count, load_time,
    		earliest_time, latest_time, file_timestamp, process_time)
    	values (pFilename, pVolumeUplink, pVolumeDownlink, pRecordCount, sysdate,
        	pEarliestTime, pLatestTime, pFileTimestamp, pProcessTimeSec);
end;


procedure SendAlert(pMessage in varchar2) is
	PGW_AGGREGATOR_MODULE	constant varchar2(20) := 'PGW aggregator';
begin
	insert into System$Alerts (module, event_date, description) values (PGW_AGGREGATOR_MODULE, sysdate, pMessage);
end;


procedure ChargePostponed(pProcIndex in integer, pProcCount in integer) is
	MAX_CHARGE_PERIOD_DAYS	constant integer := 60;
	PROCESS_DUMP_SIZE 		constant integer := 1000;
    vFound boolean;
    vRate number;
begin
	loop
    	vFound := false;
        if ChargingAllowed = 0 then
        	exit;
        end if;
        for S in (select /*+ index(m ix_mobile_session_status)*/ rowid from Mobile_Session m
            where start_time > sysdate - MAX_CHARGE_PERIOD_DAYS
                and status = SESSION_STATUS_READY_TO_CHARGE
                and mod(imsi, pProcCount) = pProcIndex
                and rownum < PROCESS_DUMP_SIZE) loop
            vFound := true;
            vRate := ChargeSession(S.rowid);
            commit;
        end loop;
        exit when not vFound;
    end loop;
end;


---------------------------------------------------
-------- TESTS  -----------------------------------
---------------------------------------------------

procedure AssertTestDatabase is
	vDatabaseKind varchar2(100);
begin
	vDatabaseKind := IRBiS.GetSystemParamValue('Database kind', sysdate);
    Test_Utl.AssertEquals(vDatabaseKind, 'test', 'Database kind check');
end;


procedure CheckTestExport is
	vCount integer;
begin
	select count(*) into vCount from (
    select s.charging_id, t.update_count, s.served_imsi, s.served_msisdn, s.apn, s.rating_group, s.serving_node_plmn_id,
        s.volume_uplink sample_uplink, s.volume_downlink sample_downlink, s.duration sample_duration,
        s.start_time sample_starttime, t.start_time test_starttime,
        (t.uplink_postponed + t.uplink_charged) test_uplink, (t.downlink_postponed + t.downlink_charged) test_downlink,
        t.duration test_duration
    from MOBILE_SESSION_SAMPLE s,
    	(select charging_id, rating_group, imsi, msisdn, plmn_id, access_point_name, min(start_time) start_time,
        	round((max(start_time + duration/86400) - min(start_time))*86400) duration,
        	sum(uplink_charged) uplink_charged, sum(uplink_postponed) uplink_postponed,
            sum(downlink_charged) downlink_charged, sum(downlink_postponed) downlink_postponed,
            sum(update_count) update_count
            from MOBILE_SESSION where start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
            group by charging_id, rating_group, imsi, msisdn, plmn_id, access_point_name
        ) t
    where  s.charging_id = t.charging_id(+) and s.served_imsi = t.imsi(+) and s.served_msisdn = t.msisdn(+)
        and s.served_msisdn = t.msisdn(+) and s.apn = t.access_point_name(+) and s.rating_group = t.rating_group(+)
        and s.serving_node_plmn_id=t.plmn_id(+)
    )
    where (nvl(sample_uplink, 0) != nvl(test_uplink, 0) or nvl(sample_downlink, 0) != nvl(test_downlink,0)
            or nvl(sample_duration, 0) != nvl(test_duration, 0) or sample_starttime != test_starttime);
	Test_Utl.AssertEquals(vCount, 0, 'Discrepancies found when comparing data volumes per session');

    -- Uplink charge check
    select count(*) into vCount from (
        select a.charging_id, a.start_time, a.duration, a.update_count, a.uplink_postponed, a.uplink_charged, a.downlink_postponed,
            a.downlink_charged, a.uplink_rate, a.downlink_rate,
            d.object_no detail_id, d.start_date, d.end_date, round((d.end_date-d.start_date)*3600*24) detail_duration, d.data_volume,
            d.data_volume* 1024*1024 detail_bytes, d.charge, d.charge/decode(d.data_volume, 0, -1, d.data_volume) detail_rate
        from Mobile_Session a, TInetDetails d where a.start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
        	and a.origin_id = d.origin_id(+) and a.uplink_detail_id = d.object_no(+)
            and a.start_time = d.start_date(+) and a.status = SESSION_STATUS_CHARGED
            and not exists (select 1 from TTrafficPacketUse u where u.service = 'INET' and u.charged_id = d.object_no))
	where duration != detail_duration or uplink_charged != detail_bytes or (uplink_rate != detail_rate and uplink_charged > 0
        	and not exists (select 1 from TTrafficPacketUse u where u.service = 'INET' and u.charged_id = detail_id));
	Test_Utl.AssertEquals(vCount, 0, 'Discrepancies found when comparing uplink charge details');

    -- Downlink charge check
	select count(*) into vCount from (
        select a.charging_id, a.start_time, a.duration, a.update_count, a.uplink_postponed, a.uplink_charged, a.downlink_postponed,
            a.downlink_charged, a.uplink_rate, a.downlink_rate,
            d.object_no detail_id, d.start_date, d.end_date, round((d.end_date-d.start_date)*3600*24) detail_duration, d.data_volume,
            d.data_volume* 1024*1024 detail_bytes, d.charge, d.charge/decode(d.data_volume, 0, -1, d.data_volume) detail_rate
        from Mobile_Session a, TInetDetails d where a.start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
        	and a.origin_id = d.origin_id(+) and a.downlink_detail_id = d.object_no(+)
            and a.start_time = d.start_date(+) and a.status = SESSION_STATUS_CHARGED
            and not exists (select 1 from TTrafficPacketUse u where u.service = 'INET' and u.charged_id = d.object_no))
	where duration != detail_duration or downlink_charged != detail_bytes or (downlink_rate != detail_rate and downlink_charged > 0
        	and not exists (select 1 from TTrafficPacketUse u where u.service = 'INET' and u.charged_id = detail_id));
	Test_Utl.AssertEquals(vCount, 0, 'Discrepancies found when comparing downlink charge details');

    -- update count check for rated traffic (rate>0): must be equal to CDR count
    select count(*) into vCount from (
    	select ms.charging_id, ms.rating_group, ms.start_time, ms.duration, ms.update_count,
            (select count(*) from Mobile_Session_Sample_Cdrs cdr
            	where cdr.charging_id = ms.charging_id and cdr.rating_group = ms.rating_group
                and cdr.datetime >= ms.start_time and cdr.datetime < ms.start_time + ms.duration/24/60/60) cdr_count
        from Mobile_Session ms
        where (ms.uplink_rate > 0 or ms.downlink_rate > 0) and ms.start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
        and ms.charging_id not in (1, 2, 3, 1420640976 /* session having rate change, excluded from test*/)
    	)
    where update_count != case when cdr_count > 1 then cdr_count - 1 else 0 end;
    Test_Utl.AssertEquals(vCount, 0, 'Discrepancies found when checking update_count');
end;


procedure ClearMobileSessions is
	vSessionID varchar2(50) := 'UWtZYGhuJutnwUKwoQdwUtVDXcxJveJk';
begin
	AssertTestDatabase;
    for MS in (select rowid from Mobile_Session where status = SESSION_STATUS_CHARGED and imsi = TEST_IMSI) loop
    	begin
    		PTelephony.RollbackRecordChargeByRowid('MOBILE_SESSION', MS.rowid, vSessionID);
        exception when others then
        	dbms_output.put_line('Exception rowid: ' || to_char(MS.rowid));
        end;
        commit;
    end loop;
    delete from Mobile_Session m where start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
    	and exists (select 1 from Mobile_Session_Sample s where s.charging_id = m.charging_id);
end;


procedure GetSubscrAttributesTest is
	vAttrs SubscriberAttributes;
    vContractID integer;
    vTarPlanID number;
    vCommonContrID number;
    vAccountID number;
    vClientID number;
begin
	for S in (select /*+ index(cm)*/ sim.sim_imsi
            from TSimcard sim/*, TContractMobile cm, TContract c
            where cm.simcard_id = sim.object_no and c.object_no = cm.object_no and c.date_out > sysdate*/
            where  sim_imsi like '25027%' and rownum < 100
            ) loop
    	begin
        	vContractID := PMobile.GetSubscriber(S.sim_imsi, null, sysdate, vTarPlanID, vCommonContrID, vAccountID, vClientID);
            vAttrs := GetSubscriberAttributes(to_number(S.sim_imsi), sysdate);

            Test_Utl.AssertEquals(vContractID, vAttrs.contractID, 'contract ID mismatch');
            Test_Utl.AssertEquals(vCommonContrID, vAttrs.contractCommonID, 'contractCommon ID mismatch');
            Test_Utl.AssertEquals(vAccountID, vAttrs.accountID, 'account ID mismatch');
            Test_Utl.AssertEquals(vClientID, vAttrs.clientID, 'client ID mismatch');
            Test_Utl.AssertEquals(vTarPlanID, vAttrs.tariffPlanID, 'tariffPlan ID mismatch');
        exception when SubscriberNotFound then
        	Test_Utl.AssertEquals(-2, vContractID, 'contract ID mismatch');
        end ;
    end loop;
end;


procedure GetNetworkAttributesTest is
	vMobileNetworkID integer;
    vMSCID integer;
    vHomeCountry integer;
    vHomeNetwork  integer;
    vHomeMSC  integer;
    vRoamingCountryID  integer;
    vCountryCode TRoamingCountry.country_code%type;
    vPseudoRoamingHubID integer;
    vAttrs NetworkAttributes;
begin
	for CDR in (
    	select * from GPRS_CDR_PGW where serving_node_address not like '185.6.%'
    	union all
        select * from GPRS_CDR_PGW where serving_node_address like '185.6.%' and rownum < 10
    ) loop
    begin
    	vMobileNetworkID := PMobile.GetMobileNetwork(CDR.serving_node_plmn_id,
        	CDR.serving_node_address, CDR.datetime, vMSCID,
    		vHomeCountry, vHomeNetwork, vHomeMSC, vRoamingCountryID, vCountryCode, vPseudoRoamingHubID);
        vAttrs := GetNetworkAttributes(CDR.serving_node_plmn_id,
        	PMobile.IPAddressToInteger(CDR.serving_node_address), CDR.datetime);
		Test_Utl.AssertEquals(vMobileNetworkID, vAttrs.mobileNetworkID, 'mobileNetworkID mismatch');
        Test_Utl.AssertEquals(vMSCID, vAttrs.mscID, 'mscID mismatch');
        Test_Utl.AssertEquals(vHomeCountry, vAttrs.homeCountry, 'vHomeCountry mismatch');
        Test_Utl.AssertEquals(vHomeNetwork, vAttrs.homeNetwork, 'homeNetwork mismatch');
        Test_Utl.AssertEquals(vHomeMSC, vAttrs.homeMSC, 'vHomeMSC mismatch');
        Test_Utl.AssertEquals(vRoamingCountryID, vAttrs.roamingCountryID, 'roamingCountryID mismatch');
        Test_Utl.AssertEquals(vCountryCode, vAttrs.countryCode, 'vCountryCode mismatch');
        Test_Utl.AssertEquals(vPseudoRoamingHubID, vAttrs.pseudoRoamingHubID, 'pseudoRoamingHubID mismatch for ' ||
        	CDR.serving_node_address);
    exception
    when NetworkNotFound then
    	Test_Utl.AssertEquals(-24, vMobileNetworkID, 'mobileNetworkID mismatch when NetworkNotFound exception caught');
    end;
    end loop;
end;


procedure SetTariffAttributesTest is
	vTariffAttrs TariffAttributes;
    vNetworkAttrs NetworkAttributes;
begin
	vNetworkAttrs.homeMSC := 1;
	vTariffAttrs := GetTariffAttributes('mms', vNetworkAttrs, sysdate);
    Test_Utl.AssertEquals(226792341 /*Моб.связь-GPRS*/, vTariffAttrs.accessTypeID, 'accessTypeID mismatch');
    Test_Utl.AssertEquals(226792058 /*Моб. связь - Бесплатный GPRS-трафик*/, vTariffAttrs.accessZoneID, 'accessZoneID mismatch');
    vTariffAttrs := GetTariffAttributes('internet', vNetworkAttrs, sysdate);
    Test_Utl.AssertEquals(226792341 /*Моб.связь-GPRS*/, vTariffAttrs.accessTypeID, 'accessTypeID mismatch');
    Test_Utl.AssertEquals(226792055 /*Моб. связь - Платный GPRS-трафик*/, vTariffAttrs.accessZoneID, 'accessZoneID mismatch');

    vNetworkAttrs.homeMSC := 0;
    vNetworkAttrs.homeNetwork := 1;
    vTariffAttrs := GetTariffAttributes('mms', vNetworkAttrs, sysdate);
    Test_Utl.AssertEquals(228020139, vTariffAttrs.accessTypeID, 'accessTypeID mismatch for homeMSC = 0 and homeNetwork = 1');

    vNetworkAttrs.homeMSC := 0;
    vNetworkAttrs.homeNetwork := 0;
    vNetworkAttrs.homeCountry := 1;
    vTariffAttrs := GetTariffAttributes('mms', vNetworkAttrs, sysdate);
    Test_Utl.AssertEquals(228020140, vTariffAttrs.accessTypeID,
    	'accessTypeID mismatch for homeMSC = 0 and homeNetwork = 0 and homeCountry = 1');

    vNetworkAttrs.homeCountry := 0;
    vTariffAttrs := GetTariffAttributes('mms', vNetworkAttrs, sysdate);
    Test_Utl.AssertEquals(228020141, vTariffAttrs.accessTypeID, 'accessTypeID mismatch for homeMSC = 0 and homeNetwork = 0
    	and homeCountry = 0');
end;

procedure GetChargedVolumeMbTest is
	vServiceID integer := 848806003;
    vSubscrAttrs SubscriberAttributes;
    vChargedVolumeMb number;
begin
	vSubscrAttrs.clientTypeID := 1;
	vSubscrAttrs.tariffPlanID := 510332513;
	vChargedVolumeMb := GetChargedVolumeMB(vServiceID, vSubscrAttrs, 50, sysdate);
    Test_Utl.AssertEquals(vChargedVolumeMb, 0, 'toll free bound');
    vChargedVolumeMb := GetChargedVolumeMB(vServiceID, vSubscrAttrs, 500, sysdate);
    Test_Utl.AssertEquals(vChargedVolumeMb, 10240/MEGABYTE_SIZE_IN_BYTES, 'less than 10K');
    vChargedVolumeMb := GetChargedVolumeMB(vServiceID, vSubscrAttrs, 10240, sysdate);
    Test_Utl.AssertEquals(vChargedVolumeMb, 10240/MEGABYTE_SIZE_IN_BYTES, 'exactly 10K');
    vChargedVolumeMb := GetChargedVolumeMB(vServiceID, vSubscrAttrs, 20000, sysdate);
    Test_Utl.AssertEquals(vChargedVolumeMb, trunc(20000/1024 + 1)*1024/MEGABYTE_SIZE_IN_BYTES, 'more than 10K');
end;

procedure GetServiceRateTest is
	vRate number;
    vSubscrAttrs SubscriberAttributes;
    vNetworkAttrs NetworkAttributes;
    vTariffAttrs TariffAttributes;
    vHomeServiceID constant integer := 619188484;
    vRoamServiceID constant integer := 667518343;
begin
	-- Home tests
    vSubscrAttrs.tariffPlanID := 828262026;
    vSubscrAttrs.contractID := 869005910;
    vNetworkAttrs.homeMSC := 1;
	vRate := GetServiceRate(vHomeServiceID, sysdate, vSubscrAttrs, vNetworkAttrs, vTariffAttrs);
    Test_Utl.AssertEquals(vRate, 5.9322034 * 1.18);

    vSubscrAttrs.contractID := 976568154;
    vRate := GetServiceRate(vHomeServiceID, sysdate, vSubscrAttrs, vNetworkAttrs, vTariffAttrs);
    Test_Utl.AssertEquals(vRate, 0.084746 * 1.18);

    vSubscrAttrs.contractID := 907409939;
    vRate := GetServiceRate(vHomeServiceID, sysdate, vSubscrAttrs, vNetworkAttrs, vTariffAttrs);
    Test_Utl.AssertEquals(vRate, 200);

    -- Roaming tests
    vNetworkAttrs.homeMSC := 0;
    vNetworkAttrs.roamingCountryID := 558101656;
	vRate := GetServiceRate(vRoamServiceID, sysdate, vSubscrAttrs, vNetworkAttrs, vTariffAttrs);
    Test_Utl.AssertEquals(vRate, 896);

    vNetworkAttrs.roamingCountryID := 458195232;
	vRate := GetServiceRate(vRoamServiceID, sysdate, vSubscrAttrs, vNetworkAttrs, vTariffAttrs);
    Test_Utl.AssertEquals(vRate, 4864);
end;


procedure ExportSessionTest is
	vChargingID constant integer := 1;
    vContractCommonID constant integer := 917622744;
    vSessionID constant varchar2(50) := 'mRsKBvztvbqNceYcenWhWVnWWNELaWkT';
    vSomeWrongIMSI constant integer := 999999;
    vCount integer;
    vStartTime date;
    vSession Mobile_Session%rowtype;
    vDetail TInetDetails%rowtype;
    vDetailUplink TInetDetails%rowtype;
    vDetailDownlink TInetDetails%rowtype;
    vRate number;
    vOldUplinkRate number;
    vRowid rowid;
    procedure ExportNext(pstartTime in date, pduration in integer, pRatingGroup in integer,
    			pvolumeUplink in integer, pvolumeDownlink in integer) is
    begin
    	vRate := ExportSession(
	   		vChargingID,
    		TEST_IMSI,
        	1234567890, --pmsisdn in integer,
            null, --pimei in varchar2,
            'test', --paccessPointName in varchar2,
            pstartTime,
            pstartTime + pduration/NUM_OF_SECONDS_IN_DAY,
            3104198665, --pservingNodeIP in integer,
            25027, --pplmnID in integer,
            pRatingGroup,
            pvolumeUplink,
            pvolumeDownlink
		);
    end;
begin
	IRBiS.SetSystemParamValue('Telephony charging allowed', '1', to_date('01.11.2009', 'dd.mm.yyyy'));
	vStartTime := to_date('26.08.2016 23:30:00','dd.mm.yyyy hh24:mi:ss');
	delete from Mobile_Session where charging_id in (1, 2, 3) and start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE;

	ExportNext(vStartTime, 180, 1, 100, 1000);
    select * into vSession from Mobile_Session
    	where charging_id = vChargingID and rating_group = 1 and status = SESSION_STATUS_CHARGED
        and start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE;
    select * into vDetailUplink from TInetDetails d
    	where start_date = vStartTime and origin_id = vSession.origin_id and object_no = vSession.uplink_detail_id;
    Test_Utl.AssertEquals(vDetailUplink.data_volume * MEGABYTE_SIZE_IN_BYTES, vSession.uplink_charged);
    Test_Utl.AssertEquals(vDetailUplink.charge, vSession.uplink_rate * vSession.uplink_charged/MEGABYTE_SIZE_IN_BYTES);
    select * into vDetailDownlink from TInetDetails d
    	where start_date = vStartTime and origin_id = vSession.origin_id and object_no = vSession.downlink_detail_id;
    Test_Utl.AssertEquals(vDetailDownlink.data_volume * MEGABYTE_SIZE_IN_BYTES, vSession.downlink_charged);
    Test_Utl.AssertEquals(vDetailDownlink.charge, vSession.downlink_rate * vSession.downlink_charged/MEGABYTE_SIZE_IN_BYTES);
    Test_Utl.Assert(vDetailUplink.service_id != vDetailDownlink.service_id);

    -- another rating group
    ExportNext(vStartTime, 180, 2, 202, 2002);
    select * into vSession from Mobile_Session
    	where charging_id = vChargingID and rating_group = 2 and status = SESSION_STATUS_CHARGED
        and start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
        and uplink_charged = 202 and downlink_charged = 2002;

    -- update volumes
    vStartTime := to_date('26.08.2016 23:20:00','dd.mm.yyyy hh24:mi:ss');
    ExportNext(vStartTime, 120, 1, 200, 2000);
    select * into vSession from Mobile_Session
    	where charging_id = vChargingID and rating_group = 1 and status = SESSION_STATUS_CHARGED
        and uplink_charged = 300 and downlink_charged = 3000 and start_time = vStartTime and duration = 780;
    select * into vDetail from TInetDetails d
    	where start_date = vStartTime and origin_id = vSession.origin_id and object_no = vSession.uplink_detail_id;
    Test_Utl.AssertEquals(vDetail.data_volume * MEGABYTE_SIZE_IN_BYTES, vSession.uplink_charged);
    Test_Utl.AssertEquals(vDetail.end_date, GetEndTime(vSession.start_time, vSession.duration));
    select * into vDetail from TInetDetails d
    	where start_date = vStartTime and origin_id = vSession.origin_id and object_no = vSession.downlink_detail_id;
    Test_Utl.AssertEquals(vDetail.data_volume * MEGABYTE_SIZE_IN_BYTES, vSession.downlink_charged);

    -- add next quant after rate change
    vStartTime := to_date('26.08.2016 23:40:00','dd.mm.yyyy hh24:mi:ss');
    ExportNext(vStartTime, 100, 1, 400, 4000);
	vOldUplinkRate := vSession.uplink_rate;
    -- check new row created:
    select * into vSession from Mobile_Session
    	where charging_id = vChargingID and rating_group = 1 and status = SESSION_STATUS_CHARGED
        and uplink_charged = 400 and downlink_charged = 4000 and start_time = vStartTime and duration = 100
        and uplink_rate != vSession.uplink_rate;
    -- check old row:
    select 1 into vCount from Mobile_Session
    	where charging_id = vChargingID and rating_group = 1 and status = SESSION_STATUS_CHARGED
        and start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
        and uplink_charged = 300 and downlink_charged = 3000 and duration = 780
        and uplink_rate = vOldUplinkRate;

    select * into vDetail from TInetDetails d
    	where start_date = vStartTime and origin_id = vSession.origin_id and object_no = vSession.uplink_detail_id;
    Test_Utl.AssertEquals(vDetail.charge, vSession.uplink_rate * vSession.uplink_charged/MEGABYTE_SIZE_IN_BYTES);
    select * into vDetail from TInetDetails d
    	where start_date = vStartTime and origin_id = vSession.origin_id and object_no = vSession.downlink_detail_id;
    Test_Utl.AssertEquals(vDetail.charge, vSession.downlink_rate * vSession.downlink_charged/MEGABYTE_SIZE_IN_BYTES);

    -- export new quant with rating group 2 with wrong network params
    vRate := ExportSession(
        vChargingID,
        TEST_IMSI,
        1234567890, --pmsisdn in integer,
        null, --pimei in varchar2,
        'test', --paccessPointName in varchar2,
        vStartTime,
        vStartTime + 100/NUM_OF_SECONDS_IN_DAY, -- pendTime
        0, --pservingNodeIP in integer,
        0, --pplmnID in integer,
        2, --pRatingGroup,
        101, --pvolumeUplink,
        1001 --pvolumeDownlink
    );
    -- check new row created having dropout status:
    select 1 into vCount from Mobile_Session
    	where charging_id = vChargingID and rating_group = 2 and status = PMobile.ERROR_NETWORK_UNKNOWN
        and uplink_postponed = 101 and downlink_postponed = 1001 and start_time = vStartTime and duration = 100;

    -- check PCharging.UpdateInetChargeImpossible processing
    ExportNext(vStartTime, 100, 3, 100, 1000);
    -- simulate closing of TCharge:
    update TCharge set bls_id = 160 where object_no in (
        select d.charge_id from Mobile_Session s, TInetDetails d
            where charging_id = vChargingID and rating_group = 3 and status = SESSION_STATUS_CHARGED
            and s.start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
            and d.origin_id = s.origin_id and d.object_no = s.uplink_detail_id and d.start_date = s.start_time);
    Test_Utl.AssertEquals(SQL%Rowcount, 1);
    ExportNext(vStartTime + 10/60/24, 50, 3, 10, 100);
    -- check old record closure:
    select 1 into vCount from Mobile_Session
    	where charging_id = vChargingID and rating_group = 3 and start_time = vStartTime and duration = 100
        and closed >= sysdate - 5/60/60/24;
    -- check new record creation:
    select 1 into vCount from Mobile_Session
    	where charging_id = vChargingID and rating_group = 3 and start_time = vStartTime + 10/60/24 and duration = 50
        and closed is null;

    -- export session of unknown subscriber
    vRate := ExportSession(
        2, --vChargingID,
        vSomeWrongIMSI,
        9876543210, --pmsisdn in integer,
        null, --pimei in varchar2,
        'wrong.imsi.test', --paccessPointName in varchar2,
        vStartTime,
        vStartTime + 150/NUM_OF_SECONDS_IN_DAY, -- pendTime
        3104198665, --pservingNodeIP in integer,
        25027, --pplmnID in integer,
        1, --pRatingGroup,
        100, --pvolumeUplink,
        1000 --pvolumeDownlink
    );
    select 1 into vCount from Mobile_Session
    	where charging_id = 2 and rating_group = 1 and start_time = vStartTime and duration = 150
        and status = PMobile.ERROR_CONTRACT_NOTFOUND and uplink_charged =0 and uplink_postponed = 100
        and downlink_charged =0 and downlink_postponed = 1000;
    vRate := ExportSession(
        2, --vChargingID,
        vSomeWrongIMSI,
        9876543210, --pmsisdn in integer,
        null, --pimei in varchar2,
        'wrong.imsi.test', --paccessPointName in varchar2,
        vStartTime + 10/60/24,
        vStartTime + 10/60/24 + 100/NUM_OF_SECONDS_IN_DAY, -- pendTime
        3104198665, --pservingNodeIP in integer,
        25027, --pplmnID in integer,
        1, --pRatingGroup,
        100, --pvolumeUplink,
        1000 --pvolumeDownlink
    );
    select 1 into vCount from Mobile_Session
    	where charging_id = 2 and rating_group = 1 and start_time = vStartTime and duration = 700
        and start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
        and status = PMobile.ERROR_CONTRACT_NOTFOUND and uplink_charged =0 and uplink_postponed = 200
        and downlink_charged =0 and downlink_postponed = 2000;

    -- export wrong rating group
    vStartTime := to_date('26.08.2016 23:30:00','dd.mm.yyyy hh24:mi:ss');
    ExportNext(vStartTime, 180, 999, 1000, 10000);
    select 1 into vCount from Mobile_Session
    	where charging_id = 1 and rating_group = 999 and start_time = vStartTime and duration = 180
        and status = PMobile.BAD_TRAFFIC_TYPE and uplink_charged =0 and uplink_postponed = 1000
        and downlink_charged =0 and downlink_postponed = 10000;
    -- update wrong rating group
    ExportNext(vStartTime + 5/60/24, 180, 999, 500, 5000);
    select 1 into vCount from Mobile_Session
    	where charging_id = 1 and rating_group = 999 and start_time = vStartTime and duration = 480
        and status = PMobile.BAD_TRAFFIC_TYPE and uplink_charged =0 and uplink_postponed = 1500
        and downlink_charged =0 and downlink_postponed = 15000;

    -- check selection of correct rate for multiple records
    vStartTime := to_date('26.08.2016 23:30:00','dd.mm.yyyy hh24:mi:ss');
    ExportNext(vStartTime, 240, 1, 300, 3000);
	select 1 into vCount from Mobile_Session
    	where charging_id = 1 and rating_group = 1 and duration = 840
        and start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
        and status = SESSION_STATUS_CHARGED and uplink_charged = 600 and downlink_charged = 6000 ;

    -- forbid charging
    IRBiS.SetSystemParamValue('Telephony charging allowed', '0', to_date('01.11.2009', 'dd.mm.yyyy'));
    vStartTime := to_date('26.08.2016 23:45:00','dd.mm.yyyy hh24:mi:ss');
    ExportNext(vStartTime, 180, 1, 100, 1000);
    select rowid into vRowid from Mobile_Session
    	where charging_id = 1 and rating_group = 1 and duration = 480
        and start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
        and status = SESSION_STATUS_READY_TO_CHARGE and uplink_charged = 400 and downlink_charged = 4000
        and uplink_postponed = 100 and downlink_postponed = 1000 ;

	select * into vSession from Mobile_Session m
    	where rowid = vRowid;
    PTelephony.RollbackRecordChargeByRowid('MOBILE_SESSION', vRowid, vSessionID);

    select 1 into vCount from Mobile_Session
    	where charging_id = 1 and rating_group = 1 and duration = 480
        and start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
        and status = PTelephony.ROLLED_BACK_CHARGE and uplink_charged = 0 and downlink_charged = 0
        and uplink_postponed = 500 and downlink_postponed = 5000
        and uplink_detail_id is null and downlink_detail_id is null;
    select count(*) into vCount from TInetDetails where origin_id = vSession.origin_id and start_date = vStartTime;
    Test_Utl.AssertEquals(vCount, 0);

    -- another rating group
    ExportNext(vStartTime, 180, 4, 104, 1004);
    select 1 into vCount from Mobile_Session
    	where charging_id = 1 and rating_group = 4 and duration = 180
        and start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
        and status = SESSION_STATUS_READY_TO_CHARGE and uplink_charged = 0 and downlink_charged = 0
        and uplink_postponed = 104 and downlink_postponed = 1004 ;

    -- allow charging
    IRBiS.SetSystemParamValue('Telephony charging allowed', '1', to_date('01.11.2009', 'dd.mm.yyyy'));
    update Mobile_Session set status = SESSION_STATUS_READY_TO_CHARGE where rowid = vRowid;
    vStartTime := to_date('26.08.2016 23:50:00','dd.mm.yyyy hh24:mi:ss');
    -- add next quant and check charging of postponed volumes
    ExportNext(vStartTime, 600, 1, 200, 2000);
    select * into vSession from Mobile_Session m
    	where charging_id = 1 and rating_group = 1 and duration = 1200
        and start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
        and status = SESSION_STATUS_CHARGED and uplink_charged = 700 and downlink_charged = 7000
        and uplink_postponed = 0 and downlink_postponed = 0 ;

	select * into vDetailUplink from TInetDetails d
    	where origin_id = vSession.origin_id and object_no = vSession.uplink_detail_id;
    Test_Utl.AssertEquals(vDetailUplink.data_volume * MEGABYTE_SIZE_IN_BYTES, vSession.uplink_charged);
    Test_Utl.AssertEquals(vDetailUplink.charge, vSession.uplink_rate * vSession.uplink_charged/MEGABYTE_SIZE_IN_BYTES);
    select * into vDetailDownlink from TInetDetails d
    	where origin_id = vSession.origin_id and object_no = vSession.downlink_detail_id;
    Test_Utl.AssertEquals(vDetailDownlink.data_volume * MEGABYTE_SIZE_IN_BYTES, vSession.downlink_charged);
    Test_Utl.AssertEquals(vDetailDownlink.charge, vSession.downlink_rate * vSession.downlink_charged/MEGABYTE_SIZE_IN_BYTES);
    Test_Utl.Assert(vDetailUplink.service_id != vDetailDownlink.service_id);

	PAbon_Rollback.RollBackCurrentCharges(vContractCommonID, vSessionID);
    select rowid into vRowid from Mobile_Session
    	where charging_id = 1 and rating_group = 1 and duration = 1200
        and start_time between SAMPLE_MIN_DATE and SAMPLE_MAX_DATE
        and status = -vContractCommonID and uplink_charged = 0 and downlink_charged = 0
        and uplink_postponed = 700 and downlink_postponed = 7000 and origin_id is null
        and uplink_detail_id is null and downlink_detail_id is null;
    select count(*) into vCount from TInetDetails where origin_id = vSession.origin_id and start_date = vStartTime;
    Test_Utl.AssertEquals(vCount, 0);

    update Mobile_Session set status = 0 where rowid = vRowid;
    vRate := ChargeSession(vRowid);
    select 1 into vCount from Mobile_Session
    	where rowid = vRowid and status = SESSION_STATUS_CHARGED;

    -- add session rating_group = 5
    vStartTime := to_date('20.08.2016','dd.mm.yyyy hh24:mi:ss');
    ExportNext(vStartTime, 300, 5, 120, 1200);

    -- add next quant after SESSION_LOOKUP_TIMEOUT
    vStartTime := vStartTime + SESSION_LOOKUP_TIMEOUT + 1;
    ExportNext(vStartTime, 700, 5, 220, 2200);

    -- new record should be created:
	select * into vSession from Mobile_Session m
    	where charging_id = 1 and rating_group = 5 and duration = 700
        and start_time = vStartTime;

    -- check months change
	vStartTime := to_date('02.09.2016', 'dd.mm.yyyy hh24:mi:ss');
    ExportNext(vStartTime, 900, 5, 320, 3200);
    -- new record should be created:
	select * into vSession from Mobile_Session m
    	where charging_id = 1 and rating_group = 5 and duration = 900
        and start_time = vStartTime;

end;


procedure RunAllTests is
begin
	AssertTestDatabase;
	GetSubscrAttributesTest;
    GetNetworkAttributesTest;
    SetTariffAttributesTest;
    GetChargedVolumeMbTest;
    GetServiceRateTest;
    ExportSessionTest;
end;

END;
/

