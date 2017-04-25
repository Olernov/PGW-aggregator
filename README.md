<h2>PGW CDR aggregator</h2>

Dependencies:
boost libraries v1.58 or higher (including binaries libboost_system.so, libboost_filesystem.so).
Oracle client (instant or complete).
g++ compiler supporting C++11 (v5 or higher)

If you want to compile ASN from sources then asn1c compiler (https://github.com/vlm/asn1c)
is also needed.

Usage:
pgw-aggregator <conf-file> [-test]

-test options runs test mode of aggregator.
Test mode could be run on test IRBiS database having sample CDR tables Mobile_Session_Sample
and Mobile_Session_Sample_Cdrs. First pgw-aggregator runs unit tests, then stored database logic
tests, then prompts to put 33 sample CDR files to INPUT_DIR. After these files are loaded
results are being compared against sample tables.
