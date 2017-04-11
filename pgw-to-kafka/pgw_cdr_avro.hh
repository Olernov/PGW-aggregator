/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef PGW_PGW_CDR_AVRO_HH_3670469445__H_
#define PGW_PGW_CDR_AVRO_HH_3670469445__H_


#include <sstream>
#include "boost/any.hpp"
#include "avro/Specific.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"

struct _pgw_cdr_avro_schema_avsc_Union__0__ {
private:
    size_t idx_;
    boost::any value_;
public:
    size_t idx() const { return idx_; }
    bool is_null() const {
        return (idx_ == 0);
    }
    void set_null() {
        idx_ = 0;
        value_ = boost::any();
    }
    std::string get_string() const;
    void set_string(const std::string& v);
    _pgw_cdr_avro_schema_avsc_Union__0__();
};

struct _pgw_cdr_avro_schema_avsc_Union__1__ {
private:
    size_t idx_;
    boost::any value_;
public:
    size_t idx() const { return idx_; }
    bool is_null() const {
        return (idx_ == 0);
    }
    void set_null() {
        idx_ = 0;
        value_ = boost::any();
    }
    int32_t get_int() const;
    void set_int(const int32_t& v);
    _pgw_cdr_avro_schema_avsc_Union__1__();
};

struct PGW_CDR {
    typedef _pgw_cdr_avro_schema_avsc_Union__0__ IMEI_t;
    typedef _pgw_cdr_avro_schema_avsc_Union__1__ SequenceNumber_t;
    int64_t IMSI;
    int64_t MSISDN;
    IMEI_t IMEI;
    int64_t PGWNodeExportTime;
    int32_t RatingGroup;
    int64_t VolumeUplink;
    int64_t VolumeDownlink;
    int64_t ChargingID;
    SequenceNumber_t SequenceNumber;
    int32_t Duration;
    std::vector<uint8_t> UserLocationInfo;
    PGW_CDR() :
        IMSI(int64_t()),
        MSISDN(int64_t()),
        IMEI(IMEI_t()),
        PGWNodeExportTime(int64_t()),
        RatingGroup(int32_t()),
        VolumeUplink(int64_t()),
        VolumeDownlink(int64_t()),
        ChargingID(int64_t()),
        SequenceNumber(SequenceNumber_t()),
        Duration(int32_t()),
        UserLocationInfo(std::vector<uint8_t>())
        { }
};

inline
std::string _pgw_cdr_avro_schema_avsc_Union__0__::get_string() const {
    if (idx_ != 1) {
        throw avro::Exception("Invalid type for union");
    }
    return boost::any_cast<std::string >(value_);
}

inline
void _pgw_cdr_avro_schema_avsc_Union__0__::set_string(const std::string& v) {
    idx_ = 1;
    value_ = v;
}

inline
int32_t _pgw_cdr_avro_schema_avsc_Union__1__::get_int() const {
    if (idx_ != 1) {
        throw avro::Exception("Invalid type for union");
    }
    return boost::any_cast<int32_t >(value_);
}

inline
void _pgw_cdr_avro_schema_avsc_Union__1__::set_int(const int32_t& v) {
    idx_ = 1;
    value_ = v;
}

inline _pgw_cdr_avro_schema_avsc_Union__0__::_pgw_cdr_avro_schema_avsc_Union__0__() : idx_(0) { }
inline _pgw_cdr_avro_schema_avsc_Union__1__::_pgw_cdr_avro_schema_avsc_Union__1__() : idx_(0) { }
namespace avro {
template<> struct codec_traits<_pgw_cdr_avro_schema_avsc_Union__0__> {
    static void encode(Encoder& e, _pgw_cdr_avro_schema_avsc_Union__0__ v) {
        e.encodeUnionIndex(v.idx());
        switch (v.idx()) {
        case 0:
            e.encodeNull();
            break;
        case 1:
            avro::encode(e, v.get_string());
            break;
        }
    }
    static void decode(Decoder& d, _pgw_cdr_avro_schema_avsc_Union__0__& v) {
        size_t n = d.decodeUnionIndex();
        if (n >= 2) { throw avro::Exception("Union index too big"); }
        switch (n) {
        case 0:
            d.decodeNull();
            v.set_null();
            break;
        case 1:
            {
                std::string vv;
                avro::decode(d, vv);
                v.set_string(vv);
            }
            break;
        }
    }
};

template<> struct codec_traits<_pgw_cdr_avro_schema_avsc_Union__1__> {
    static void encode(Encoder& e, _pgw_cdr_avro_schema_avsc_Union__1__ v) {
        e.encodeUnionIndex(v.idx());
        switch (v.idx()) {
        case 0:
            e.encodeNull();
            break;
        case 1:
            avro::encode(e, v.get_int());
            break;
        }
    }
    static void decode(Decoder& d, _pgw_cdr_avro_schema_avsc_Union__1__& v) {
        size_t n = d.decodeUnionIndex();
        if (n >= 2) { throw avro::Exception("Union index too big"); }
        switch (n) {
        case 0:
            d.decodeNull();
            v.set_null();
            break;
        case 1:
            {
                int32_t vv;
                avro::decode(d, vv);
                v.set_int(vv);
            }
            break;
        }
    }
};

template<> struct codec_traits<PGW_CDR> {
    static void encode(Encoder& e, const PGW_CDR& v) {
        avro::encode(e, v.IMSI);
        avro::encode(e, v.MSISDN);
        avro::encode(e, v.IMEI);
        avro::encode(e, v.PGWNodeExportTime);
        avro::encode(e, v.RatingGroup);
        avro::encode(e, v.VolumeUplink);
        avro::encode(e, v.VolumeDownlink);
        avro::encode(e, v.ChargingID);
        avro::encode(e, v.SequenceNumber);
        avro::encode(e, v.Duration);
        avro::encode(e, v.UserLocationInfo);
    }
    static void decode(Decoder& d, PGW_CDR& v) {
        if (avro::ResolvingDecoder *rd =
            dynamic_cast<avro::ResolvingDecoder *>(&d)) {
            const std::vector<size_t> fo = rd->fieldOrder();
            for (std::vector<size_t>::const_iterator it = fo.begin();
                it != fo.end(); ++it) {
                switch (*it) {
                case 0:
                    avro::decode(d, v.IMSI);
                    break;
                case 1:
                    avro::decode(d, v.MSISDN);
                    break;
                case 2:
                    avro::decode(d, v.IMEI);
                    break;
                case 3:
                    avro::decode(d, v.PGWNodeExportTime);
                    break;
                case 4:
                    avro::decode(d, v.RatingGroup);
                    break;
                case 5:
                    avro::decode(d, v.VolumeUplink);
                    break;
                case 6:
                    avro::decode(d, v.VolumeDownlink);
                    break;
                case 7:
                    avro::decode(d, v.ChargingID);
                    break;
                case 8:
                    avro::decode(d, v.SequenceNumber);
                    break;
                case 9:
                    avro::decode(d, v.Duration);
                    break;
                case 10:
                    avro::decode(d, v.UserLocationInfo);
                    break;
                default:
                    break;
                }
            }
        } else {
            avro::decode(d, v.IMSI);
            avro::decode(d, v.MSISDN);
            avro::decode(d, v.IMEI);
            avro::decode(d, v.PGWNodeExportTime);
            avro::decode(d, v.RatingGroup);
            avro::decode(d, v.VolumeUplink);
            avro::decode(d, v.VolumeDownlink);
            avro::decode(d, v.ChargingID);
            avro::decode(d, v.SequenceNumber);
            avro::decode(d, v.Duration);
            avro::decode(d, v.UserLocationInfo);
        }
    }
};

}
#endif
