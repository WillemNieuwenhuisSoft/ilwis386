// error codes
#define SDErrorNr                      ILWSF("ILWISGEN", 1)
#define SDErrUnknown                   ILWSF("ILWISGEN", 2)
#define SDErrInsuffMem                 ILWSF("ILWISGEN", 3)
#define SDErrCreate                    ILWSF("ILWISGEN", 4)
#define SDErrFind                      ILWSF("ILWISGEN", 5)
#define SDErrRead                      ILWSF("ILWISGEN", 6)
#define SDErrWrite                     ILWSF("ILWISGEN", 7)
#define SDErrAccess                    ILWSF("ILWISGEN", 8)
#define SDErrDiskFull                  ILWSF("ILWISGEN", 9)
#define SDErrWrongType                 ILWSF("ILWISGEN", 10)
#define SDErrBinaryVersionNotSupported_S ILWSF("ILWISGEN", 11)
#define SDErrModuleVersionNotSupported_S ILWSF("ILWISGEN", 12)
#define SDErrODFVersionNotSupported_S  ILWSF("ILWISGEN", 13)
// 'data type' names
#define SDFile                         ILWSF("ILWISGEN", 100)
#define SDBuffer                       ILWSF("ILWISGEN", 101)
#define SDMap                          ILWSF("ILWISGEN", 102)
#define SDTable                        ILWSF("ILWISGEN", 103)
#define SDColumn                       ILWSF("ILWISGEN", 104)
#define SDSegment                      ILWSF("ILWISGEN", 105)
#define SDSegmentMap                   ILWSF("ILWISGEN", 106)
#define SDPolygon                      ILWSF("ILWISGEN", 107)
#define SDPolygonMap                   ILWSF("ILWISGEN", 108)
#define SDLut                          ILWSF("ILWISGEN", 109)
#define SDHist                         ILWSF("ILWISGEN", 110)
#define SDDomain                       ILWSF("ILWISGEN", 111)
#define SDProjections                  ILWSF("ILWISGEN", 112)
#define SDGeoRef                       ILWSF("ILWISGEN", 113)
#define SDPointMap                     ILWSF("ILWISGEN", 114)
// general user interface strings
#define SDUiInpMap                     ILWSF("ILWISGEN", 200)
#define SDUiOutMap                     ILWSF("ILWISGEN", 201)
#define SDUiRange                      ILWSF("ILWISGEN", 202)
#define SDUiInpRange                   ILWSF("ILWISGEN", 203)
#define SDUiOutRange                   ILWSF("ILWISGEN", 204)
#define SDUiPercentage                 ILWSF("ILWISGEN", 205)
// form buttons
#define SDUiCancel                     ILWSF("ILWISGEN", 300)
#define SDUiOK                         ILWSF("ILWISGEN", 301)
#define SDUiHelp                       ILWSF("ILWISGEN", 302)
#define SDUiStop                       ILWSF("ILWISGEN", 303)
// tranquilize
#define SDCalcHist                     ILWSF("ILWISGEN", 500)
// principal components
#define SDPcInitPC                     ILWSF("ILWISGEN", 600)
#define SDPcRdErrNewWarn               ILWSF("ILWISGEN", 601)
#define SDPcCalcVarcovMat              ILWSF("ILWISGEN", 602)
#define SDPcCalcCorrMat                ILWSF("ILWISGEN", 603)
#define SDPcCalcVarcovEigMat           ILWSF("ILWISGEN", 604)
#define SDPcCalcCorrEigMat             ILWSF("ILWISGEN", 605)
#define SDPcmp                         ILWSF("ILWISGEN", 606)
// matrix
#define SDMatSingular                  ILWSF("ILWISGEN", 700)
#define SDMatNonSymmetric              ILWSF("ILWISGEN", 701)
#define SDMatNoEigenValues             ILWSF("ILWISGEN", 702)
