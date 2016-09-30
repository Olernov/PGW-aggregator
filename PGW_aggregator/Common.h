#pragma once

const unsigned long long emptyValueULL = -1;
const unsigned long emptyValueUL = -1;

const int MAX_PGW_QUEUES = 16;


enum ExportResult
{
	erSuccess = 0,
	erDBError
};

enum AggregationTestType
{
	noTest = 0,
	perFileTest = 1,
	totalTest = 2
};
