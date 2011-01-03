

static const struct field_t _TransportAddress_ipAddress[] = {	
	{FNAME("ip") OCTSTR, FIXD, 4, 0, DECODE,
	 offsetof(TransportAddress_ipAddress, ip), NULL},
	{FNAME("port") INT, WORD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _TransportAddress_ipSourceRoute_route[] = {	
	{FNAME("item") OCTSTR, FIXD, 4, 0, SKIP, 0, NULL},
};

static const struct field_t _TransportAddress_ipSourceRoute_routing[] = {	
	{FNAME("strict") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("loose") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _TransportAddress_ipSourceRoute[] = {	
	{FNAME("ip") OCTSTR, FIXD, 4, 0, SKIP, 0, NULL},
	{FNAME("port") INT, WORD, 0, 0, SKIP, 0, NULL},
	{FNAME("route") SEQOF, SEMI, 0, 0, SKIP, 0,
	 _TransportAddress_ipSourceRoute_route},
	{FNAME("routing") CHOICE, 1, 2, 2, SKIP | EXT, 0,
	 _TransportAddress_ipSourceRoute_routing},
};

static const struct field_t _TransportAddress_ipxAddress[] = {	
	{FNAME("node") OCTSTR, FIXD, 6, 0, SKIP, 0, NULL},
	{FNAME("netnum") OCTSTR, FIXD, 4, 0, SKIP, 0, NULL},
	{FNAME("port") OCTSTR, FIXD, 2, 0, SKIP, 0, NULL},
};

static const struct field_t _TransportAddress_ip6Address[] = {	
	{FNAME("ip") OCTSTR, FIXD, 16, 0, DECODE,
	 offsetof(TransportAddress_ip6Address, ip), NULL},
	{FNAME("port") INT, WORD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H221NonStandard[] = {	
	{FNAME("t35CountryCode") INT, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("t35Extension") INT, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("manufacturerCode") INT, WORD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _NonStandardIdentifier[] = {	
	{FNAME("object") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("h221NonStandard") SEQ, 0, 3, 3, SKIP | EXT, 0,
	 _H221NonStandard},
};

static const struct field_t _NonStandardParameter[] = {	
	{FNAME("nonStandardIdentifier") CHOICE, 1, 2, 2, SKIP | EXT, 0,
	 _NonStandardIdentifier},
	{FNAME("data") OCTSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _TransportAddress[] = {	
	{FNAME("ipAddress") SEQ, 0, 2, 2, DECODE,
	 offsetof(TransportAddress, ipAddress), _TransportAddress_ipAddress},
	{FNAME("ipSourceRoute") SEQ, 0, 4, 4, SKIP | EXT, 0,
	 _TransportAddress_ipSourceRoute},
	{FNAME("ipxAddress") SEQ, 0, 3, 3, SKIP, 0,
	 _TransportAddress_ipxAddress},
	{FNAME("ip6Address") SEQ, 0, 2, 2, DECODE | EXT,
	 offsetof(TransportAddress, ip6Address),
	 _TransportAddress_ip6Address},
	{FNAME("netBios") OCTSTR, FIXD, 16, 0, SKIP, 0, NULL},
	{FNAME("nsap") OCTSTR, 5, 1, 0, SKIP, 0, NULL},
	{FNAME("nonStandardAddress") SEQ, 0, 2, 2, SKIP, 0,
	 _NonStandardParameter},
};

static const struct field_t _AliasAddress[] = {	
	{FNAME("dialedDigits") NUMDGT, 7, 1, 0, SKIP, 0, NULL},
	{FNAME("h323-ID") BMPSTR, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("url-ID") IA5STR, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("transportID") CHOICE, 3, 7, 7, SKIP | EXT, 0, NULL},
	{FNAME("email-ID") IA5STR, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("partyNumber") CHOICE, 3, 5, 5, SKIP | EXT, 0, NULL},
	{FNAME("mobileUIM") CHOICE, 1, 2, 2, SKIP | EXT, 0, NULL},
};

static const struct field_t _Setup_UUIE_sourceAddress[] = {	
	{FNAME("item") CHOICE, 1, 2, 7, SKIP | EXT, 0, _AliasAddress},
};

static const struct field_t _VendorIdentifier[] = {	
	{FNAME("vendor") SEQ, 0, 3, 3, SKIP | EXT, 0, _H221NonStandard},
	{FNAME("productId") OCTSTR, BYTE, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("versionId") OCTSTR, BYTE, 1, 0, SKIP | OPT, 0, NULL},
};

static const struct field_t _GatekeeperInfo[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
};

static const struct field_t _H310Caps[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("dataRatesSupported") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("supportedPrefixes") SEQOF, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H320Caps[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("dataRatesSupported") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("supportedPrefixes") SEQOF, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H321Caps[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("dataRatesSupported") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("supportedPrefixes") SEQOF, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H322Caps[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("dataRatesSupported") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("supportedPrefixes") SEQOF, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H323Caps[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("dataRatesSupported") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("supportedPrefixes") SEQOF, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H324Caps[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("dataRatesSupported") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("supportedPrefixes") SEQOF, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _VoiceCaps[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("dataRatesSupported") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("supportedPrefixes") SEQOF, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _T120OnlyCaps[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("dataRatesSupported") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("supportedPrefixes") SEQOF, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _SupportedProtocols[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP, 0,
	 _NonStandardParameter},
	{FNAME("h310") SEQ, 1, 1, 3, SKIP | EXT, 0, _H310Caps},
	{FNAME("h320") SEQ, 1, 1, 3, SKIP | EXT, 0, _H320Caps},
	{FNAME("h321") SEQ, 1, 1, 3, SKIP | EXT, 0, _H321Caps},
	{FNAME("h322") SEQ, 1, 1, 3, SKIP | EXT, 0, _H322Caps},
	{FNAME("h323") SEQ, 1, 1, 3, SKIP | EXT, 0, _H323Caps},
	{FNAME("h324") SEQ, 1, 1, 3, SKIP | EXT, 0, _H324Caps},
	{FNAME("voice") SEQ, 1, 1, 3, SKIP | EXT, 0, _VoiceCaps},
	{FNAME("t120-only") SEQ, 1, 1, 3, SKIP | EXT, 0, _T120OnlyCaps},
	{FNAME("nonStandardProtocol") SEQ, 2, 3, 3, SKIP | EXT, 0, NULL},
	{FNAME("t38FaxAnnexbOnly") SEQ, 2, 5, 5, SKIP | EXT, 0, NULL},
};

static const struct field_t _GatewayInfo_protocol[] = {	
	{FNAME("item") CHOICE, 4, 9, 11, SKIP | EXT, 0, _SupportedProtocols},
};

static const struct field_t _GatewayInfo[] = {	
	{FNAME("protocol") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _GatewayInfo_protocol},
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
};

static const struct field_t _McuInfo[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("protocol") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
};

static const struct field_t _TerminalInfo[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
};

static const struct field_t _EndpointType[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("vendor") SEQ, 2, 3, 3, SKIP | EXT | OPT, 0,
	 _VendorIdentifier},
	{FNAME("gatekeeper") SEQ, 1, 1, 1, SKIP | EXT | OPT, 0,
	 _GatekeeperInfo},
	{FNAME("gateway") SEQ, 2, 2, 2, SKIP | EXT | OPT, 0, _GatewayInfo},
	{FNAME("mcu") SEQ, 1, 1, 2, SKIP | EXT | OPT, 0, _McuInfo},
	{FNAME("terminal") SEQ, 1, 1, 1, SKIP | EXT | OPT, 0, _TerminalInfo},
	{FNAME("mc") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("undefinedNode") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("set") BITSTR, FIXD, 32, 0, SKIP | OPT, 0, NULL},
	{FNAME("supportedTunnelledProtocols") SEQOF, SEMI, 0, 0, SKIP | OPT,
	 0, NULL},
};

static const struct field_t _Setup_UUIE_destinationAddress[] = {	
	{FNAME("item") CHOICE, 1, 2, 7, SKIP | EXT, 0, _AliasAddress},
};

static const struct field_t _Setup_UUIE_destExtraCallInfo[] = {	
	{FNAME("item") CHOICE, 1, 2, 7, SKIP | EXT, 0, _AliasAddress},
};

static const struct field_t _Setup_UUIE_destExtraCRV[] = {	
	{FNAME("item") INT, WORD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _Setup_UUIE_conferenceGoal[] = {	
	{FNAME("create") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("join") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("invite") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("capability-negotiation") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("callIndependentSupplementaryService") NUL, FIXD, 0, 0, SKIP,
	 0, NULL},
};

static const struct field_t _Q954Details[] = {	
	{FNAME("conferenceCalling") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("threePartyService") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _QseriesOptions[] = {	
	{FNAME("q932Full") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("q951Full") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("q952Full") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("q953Full") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("q955Full") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("q956Full") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("q957Full") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("q954Info") SEQ, 0, 2, 2, SKIP | EXT, 0, _Q954Details},
};

static const struct field_t _CallType[] = {	
	{FNAME("pointToPoint") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("oneToN") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("nToOne") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("nToN") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H245_NonStandardIdentifier_h221NonStandard[] = {	
	{FNAME("t35CountryCode") INT, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("t35Extension") INT, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("manufacturerCode") INT, WORD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H245_NonStandardIdentifier[] = {	
	{FNAME("object") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("h221NonStandard") SEQ, 0, 3, 3, SKIP, 0,
	 _H245_NonStandardIdentifier_h221NonStandard},
};

static const struct field_t _H245_NonStandardParameter[] = {	
	{FNAME("nonStandardIdentifier") CHOICE, 1, 2, 2, SKIP, 0,
	 _H245_NonStandardIdentifier},
	{FNAME("data") OCTSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H261VideoCapability[] = {	
	{FNAME("qcifMPI") INT, 2, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("cifMPI") INT, 2, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("temporalSpatialTradeOffCapability") BOOL, FIXD, 0, 0, SKIP, 0,
	 NULL},
	{FNAME("maxBitRate") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("stillImageTransmission") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("videoBadMBsCap") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H262VideoCapability[] = {	
	{FNAME("profileAndLevel-SPatML") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("profileAndLevel-MPatLL") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("profileAndLevel-MPatML") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("profileAndLevel-MPatH-14") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("profileAndLevel-MPatHL") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("profileAndLevel-SNRatLL") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("profileAndLevel-SNRatML") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("profileAndLevel-SpatialatH-14") BOOL, FIXD, 0, 0, SKIP, 0,
	 NULL},
	{FNAME("profileAndLevel-HPatML") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("profileAndLevel-HPatH-14") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("profileAndLevel-HPatHL") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("videoBitRate") INT, CONS, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("vbvBufferSize") INT, CONS, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("samplesPerLine") INT, WORD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("linesPerFrame") INT, WORD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("framesPerSecond") INT, 4, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("luminanceSampleRate") INT, CONS, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("videoBadMBsCap") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H263VideoCapability[] = {	
	{FNAME("sqcifMPI") INT, 5, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("qcifMPI") INT, 5, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("cifMPI") INT, 5, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("cif4MPI") INT, 5, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("cif16MPI") INT, 5, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("maxBitRate") INT, CONS, 1, 0, SKIP, 0, NULL},
	{FNAME("unrestrictedVector") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("arithmeticCoding") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("advancedPrediction") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("pbFrames") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("temporalSpatialTradeOffCapability") BOOL, FIXD, 0, 0, SKIP, 0,
	 NULL},
	{FNAME("hrd-B") INT, CONS, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("bppMaxKb") INT, WORD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("slowSqcifMPI") INT, WORD, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("slowQcifMPI") INT, WORD, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("slowCifMPI") INT, WORD, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("slowCif4MPI") INT, WORD, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("slowCif16MPI") INT, WORD, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("errorCompensation") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("enhancementLayerInfo") SEQ, 3, 4, 4, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("h263Options") SEQ, 5, 29, 31, SKIP | EXT | OPT, 0, NULL},
};

static const struct field_t _IS11172VideoCapability[] = {	
	{FNAME("constrainedBitstream") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("videoBitRate") INT, CONS, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("vbvBufferSize") INT, CONS, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("samplesPerLine") INT, WORD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("linesPerFrame") INT, WORD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("pictureRate") INT, 4, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("luminanceSampleRate") INT, CONS, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("videoBadMBsCap") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _VideoCapability[] = {	
	{FNAME("nonStandard") SEQ, 0, 2, 2, SKIP, 0,
	 _H245_NonStandardParameter},
	{FNAME("h261VideoCapability") SEQ, 2, 5, 6, SKIP | EXT, 0,
	 _H261VideoCapability},
	{FNAME("h262VideoCapability") SEQ, 6, 17, 18, SKIP | EXT, 0,
	 _H262VideoCapability},
	{FNAME("h263VideoCapability") SEQ, 7, 13, 21, SKIP | EXT, 0,
	 _H263VideoCapability},
	{FNAME("is11172VideoCapability") SEQ, 6, 7, 8, SKIP | EXT, 0,
	 _IS11172VideoCapability},
	{FNAME("genericVideoCapability") SEQ, 5, 6, 6, SKIP | EXT, 0, NULL},
};

static const struct field_t _AudioCapability_g7231[] = {	
	{FNAME("maxAl-sduAudioFrames") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("silenceSuppression") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _IS11172AudioCapability[] = {	
	{FNAME("audioLayer1") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioLayer2") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioLayer3") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioSampling32k") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioSampling44k1") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioSampling48k") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("singleChannel") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("twoChannels") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("bitRate") INT, WORD, 1, 0, SKIP, 0, NULL},
};

static const struct field_t _IS13818AudioCapability[] = {	
	{FNAME("audioLayer1") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioLayer2") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioLayer3") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioSampling16k") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioSampling22k05") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioSampling24k") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioSampling32k") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioSampling44k1") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("audioSampling48k") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("singleChannel") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("twoChannels") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("threeChannels2-1") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("threeChannels3-0") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("fourChannels2-0-2-0") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("fourChannels2-2") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("fourChannels3-1") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("fiveChannels3-0-2-0") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("fiveChannels3-2") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("lowFrequencyEnhancement") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("multilingual") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("bitRate") INT, WORD, 1, 0, SKIP, 0, NULL},
};

static const struct field_t _AudioCapability[] = {	
	{FNAME("nonStandard") SEQ, 0, 2, 2, SKIP, 0,
	 _H245_NonStandardParameter},
	{FNAME("g711Alaw64k") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("g711Alaw56k") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("g711Ulaw64k") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("g711Ulaw56k") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("g722-64k") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("g722-56k") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("g722-48k") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("g7231") SEQ, 0, 2, 2, SKIP, 0, _AudioCapability_g7231},
	{FNAME("g728") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("g729") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("g729AnnexA") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("is11172AudioCapability") SEQ, 0, 9, 9, SKIP | EXT, 0,
	 _IS11172AudioCapability},
	{FNAME("is13818AudioCapability") SEQ, 0, 21, 21, SKIP | EXT, 0,
	 _IS13818AudioCapability},
	{FNAME("g729wAnnexB") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("g729AnnexAwAnnexB") INT, BYTE, 1, 0, SKIP, 0, NULL},
	{FNAME("g7231AnnexCCapability") SEQ, 1, 3, 3, SKIP | EXT, 0, NULL},
	{FNAME("gsmFullRate") SEQ, 0, 3, 3, SKIP | EXT, 0, NULL},
	{FNAME("gsmHalfRate") SEQ, 0, 3, 3, SKIP | EXT, 0, NULL},
	{FNAME("gsmEnhancedFullRate") SEQ, 0, 3, 3, SKIP | EXT, 0, NULL},
	{FNAME("genericAudioCapability") SEQ, 5, 6, 6, SKIP | EXT, 0, NULL},
	{FNAME("g729Extensions") SEQ, 1, 8, 8, SKIP | EXT, 0, NULL},
};

static const struct field_t _DataProtocolCapability[] = {	
	{FNAME("nonStandard") SEQ, 0, 2, 2, SKIP, 0,
	 _H245_NonStandardParameter},
	{FNAME("v14buffered") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("v42lapm") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("hdlcFrameTunnelling") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("h310SeparateVCStack") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("h310SingleVCStack") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("transparent") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("segmentationAndReassembly") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("hdlcFrameTunnelingwSAR") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("v120") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("separateLANStack") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("v76wCompression") CHOICE, 2, 3, 3, SKIP | EXT, 0, NULL},
	{FNAME("tcp") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("udp") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _T84Profile_t84Restricted[] = {	
	{FNAME("qcif") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("cif") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("ccir601Seq") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("ccir601Prog") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("hdtvSeq") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("hdtvProg") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("g3FacsMH200x100") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("g3FacsMH200x200") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("g4FacsMMR200x100") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("g4FacsMMR200x200") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("jbig200x200Seq") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("jbig200x200Prog") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("jbig300x300Seq") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("jbig300x300Prog") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("digPhotoLow") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("digPhotoMedSeq") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("digPhotoMedProg") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("digPhotoHighSeq") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("digPhotoHighProg") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _T84Profile[] = {	
	{FNAME("t84Unrestricted") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("t84Restricted") SEQ, 0, 19, 19, SKIP | EXT, 0,
	 _T84Profile_t84Restricted},
};

static const struct field_t _DataApplicationCapability_application_t84[] = {	
	{FNAME("t84Protocol") CHOICE, 3, 7, 14, SKIP | EXT, 0,
	 _DataProtocolCapability},
	{FNAME("t84Profile") CHOICE, 1, 2, 2, SKIP, 0, _T84Profile},
};

static const struct field_t _DataApplicationCapability_application_nlpid[] = {	
	{FNAME("nlpidProtocol") CHOICE, 3, 7, 14, SKIP | EXT, 0,
	 _DataProtocolCapability},
	{FNAME("nlpidData") OCTSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _DataApplicationCapability_application[] = {	
	{FNAME("nonStandard") SEQ, 0, 2, 2, SKIP, 0,
	 _H245_NonStandardParameter},
	{FNAME("t120") CHOICE, 3, 7, 14, DECODE | EXT,
	 offsetof(DataApplicationCapability_application, t120),
	 _DataProtocolCapability},
	{FNAME("dsm-cc") CHOICE, 3, 7, 14, SKIP | EXT, 0,
	 _DataProtocolCapability},
	{FNAME("userData") CHOICE, 3, 7, 14, SKIP | EXT, 0,
	 _DataProtocolCapability},
	{FNAME("t84") SEQ, 0, 2, 2, SKIP, 0,
	 _DataApplicationCapability_application_t84},
	{FNAME("t434") CHOICE, 3, 7, 14, SKIP | EXT, 0,
	 _DataProtocolCapability},
	{FNAME("h224") CHOICE, 3, 7, 14, SKIP | EXT, 0,
	 _DataProtocolCapability},
	{FNAME("nlpid") SEQ, 0, 2, 2, SKIP, 0,
	 _DataApplicationCapability_application_nlpid},
	{FNAME("dsvdControl") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("h222DataPartitioning") CHOICE, 3, 7, 14, SKIP | EXT, 0,
	 _DataProtocolCapability},
	{FNAME("t30fax") CHOICE, 3, 7, 14, SKIP | EXT, 0, NULL},
	{FNAME("t140") CHOICE, 3, 7, 14, SKIP | EXT, 0, NULL},
	{FNAME("t38fax") SEQ, 0, 2, 2, SKIP, 0, NULL},
	{FNAME("genericDataCapability") SEQ, 5, 6, 6, SKIP | EXT, 0, NULL},
};

static const struct field_t _DataApplicationCapability[] = {	
	{FNAME("application") CHOICE, 4, 10, 14, DECODE | EXT,
	 offsetof(DataApplicationCapability, application),
	 _DataApplicationCapability_application},
	{FNAME("maxBitRate") INT, CONS, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _EncryptionMode[] = {	
	{FNAME("nonStandard") SEQ, 0, 2, 2, SKIP, 0,
	 _H245_NonStandardParameter},
	{FNAME("h233Encryption") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _DataType[] = {	
	{FNAME("nonStandard") SEQ, 0, 2, 2, SKIP, 0,
	 _H245_NonStandardParameter},
	{FNAME("nullData") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("videoData") CHOICE, 3, 5, 6, SKIP | EXT, 0, _VideoCapability},
	{FNAME("audioData") CHOICE, 4, 14, 22, SKIP | EXT, 0,
	 _AudioCapability},
	{FNAME("data") SEQ, 0, 2, 2, DECODE | EXT, offsetof(DataType, data),
	 _DataApplicationCapability},
	{FNAME("encryptionData") CHOICE, 1, 2, 2, SKIP | EXT, 0,
	 _EncryptionMode},
	{FNAME("h235Control") SEQ, 0, 2, 2, SKIP, 0, NULL},
	{FNAME("h235Media") SEQ, 0, 2, 2, SKIP | EXT, 0, NULL},
	{FNAME("multiplexedStream") SEQ, 0, 2, 2, SKIP | EXT, 0, NULL},
};

static const struct field_t _H222LogicalChannelParameters[] = {	
	{FNAME("resourceID") INT, WORD, 0, 0, SKIP, 0, NULL},
	{FNAME("subChannelID") INT, WORD, 0, 0, SKIP, 0, NULL},
	{FNAME("pcr-pid") INT, WORD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("programDescriptors") OCTSTR, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("streamDescriptors") OCTSTR, SEMI, 0, 0, SKIP | OPT, 0, NULL},
};

static const struct field_t _H223LogicalChannelParameters_adaptationLayerType_al3[] = {	
	{FNAME("controlFieldOctets") INT, 2, 0, 0, SKIP, 0, NULL},
	{FNAME("sendBufferSize") INT, CONS, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H223LogicalChannelParameters_adaptationLayerType[] = {	
	{FNAME("nonStandard") SEQ, 0, 2, 2, SKIP, 0,
	 _H245_NonStandardParameter},
	{FNAME("al1Framed") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("al1NotFramed") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("al2WithoutSequenceNumbers") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("al2WithSequenceNumbers") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("al3") SEQ, 0, 2, 2, SKIP, 0,
	 _H223LogicalChannelParameters_adaptationLayerType_al3},
	{FNAME("al1M") SEQ, 0, 7, 8, SKIP | EXT, 0, NULL},
	{FNAME("al2M") SEQ, 0, 2, 2, SKIP | EXT, 0, NULL},
	{FNAME("al3M") SEQ, 0, 5, 6, SKIP | EXT, 0, NULL},
};

static const struct field_t _H223LogicalChannelParameters[] = {	
	{FNAME("adaptationLayerType") CHOICE, 3, 6, 9, SKIP | EXT, 0,
	 _H223LogicalChannelParameters_adaptationLayerType},
	{FNAME("segmentableFlag") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _CRCLength[] = {	
	{FNAME("crc8bit") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("crc16bit") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("crc32bit") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _V76HDLCParameters[] = {	
	{FNAME("crcLength") CHOICE, 2, 3, 3, SKIP | EXT, 0, _CRCLength},
	{FNAME("n401") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("loopbackTestProcedure") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _V76LogicalChannelParameters_suspendResume[] = {	
	{FNAME("noSuspendResume") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("suspendResumewAddress") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("suspendResumewoAddress") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _V76LogicalChannelParameters_mode_eRM_recovery[] = {	
	{FNAME("rej") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("sREJ") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("mSREJ") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _V76LogicalChannelParameters_mode_eRM[] = {	
	{FNAME("windowSize") INT, 7, 1, 0, SKIP, 0, NULL},
	{FNAME("recovery") CHOICE, 2, 3, 3, SKIP | EXT, 0,
	 _V76LogicalChannelParameters_mode_eRM_recovery},
};

static const struct field_t _V76LogicalChannelParameters_mode[] = {	
	{FNAME("eRM") SEQ, 0, 2, 2, SKIP | EXT, 0,
	 _V76LogicalChannelParameters_mode_eRM},
	{FNAME("uNERM") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _V75Parameters[] = {	
	{FNAME("audioHeaderPresent") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _V76LogicalChannelParameters[] = {	
	{FNAME("hdlcParameters") SEQ, 0, 3, 3, SKIP | EXT, 0,
	 _V76HDLCParameters},
	{FNAME("suspendResume") CHOICE, 2, 3, 3, SKIP | EXT, 0,
	 _V76LogicalChannelParameters_suspendResume},
	{FNAME("uIH") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("mode") CHOICE, 1, 2, 2, SKIP | EXT, 0,
	 _V76LogicalChannelParameters_mode},
	{FNAME("v75Parameters") SEQ, 0, 1, 1, SKIP | EXT, 0, _V75Parameters},
};

static const struct field_t _H2250LogicalChannelParameters_nonStandard[] = {	
	{FNAME("item") SEQ, 0, 2, 2, SKIP, 0, _H245_NonStandardParameter},
};

static const struct field_t _UnicastAddress_iPAddress[] = {	
	{FNAME("network") OCTSTR, FIXD, 4, 0, DECODE,
	 offsetof(UnicastAddress_iPAddress, network), NULL},
	{FNAME("tsapIdentifier") INT, WORD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _UnicastAddress_iPXAddress[] = {	
	{FNAME("node") OCTSTR, FIXD, 6, 0, SKIP, 0, NULL},
	{FNAME("netnum") OCTSTR, FIXD, 4, 0, SKIP, 0, NULL},
	{FNAME("tsapIdentifier") OCTSTR, FIXD, 2, 0, SKIP, 0, NULL},
};

static const struct field_t _UnicastAddress_iP6Address[] = {	
	{FNAME("network") OCTSTR, FIXD, 16, 0, DECODE,
	 offsetof(UnicastAddress_iP6Address, network), NULL},
	{FNAME("tsapIdentifier") INT, WORD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _UnicastAddress_iPSourceRouteAddress_routing[] = {	
	{FNAME("strict") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("loose") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _UnicastAddress_iPSourceRouteAddress_route[] = {	
	{FNAME("item") OCTSTR, FIXD, 4, 0, SKIP, 0, NULL},
};

static const struct field_t _UnicastAddress_iPSourceRouteAddress[] = {	
	{FNAME("routing") CHOICE, 1, 2, 2, SKIP, 0,
	 _UnicastAddress_iPSourceRouteAddress_routing},
	{FNAME("network") OCTSTR, FIXD, 4, 0, SKIP, 0, NULL},
	{FNAME("tsapIdentifier") INT, WORD, 0, 0, SKIP, 0, NULL},
	{FNAME("route") SEQOF, SEMI, 0, 0, SKIP, 0,
	 _UnicastAddress_iPSourceRouteAddress_route},
};

static const struct field_t _UnicastAddress[] = {	
	{FNAME("iPAddress") SEQ, 0, 2, 2, DECODE | EXT,
	 offsetof(UnicastAddress, iPAddress), _UnicastAddress_iPAddress},
	{FNAME("iPXAddress") SEQ, 0, 3, 3, SKIP | EXT, 0,
	 _UnicastAddress_iPXAddress},
	{FNAME("iP6Address") SEQ, 0, 2, 2, DECODE | EXT,
	 offsetof(UnicastAddress, iP6Address), _UnicastAddress_iP6Address},
	{FNAME("netBios") OCTSTR, FIXD, 16, 0, SKIP, 0, NULL},
	{FNAME("iPSourceRouteAddress") SEQ, 0, 4, 4, SKIP | EXT, 0,
	 _UnicastAddress_iPSourceRouteAddress},
	{FNAME("nsap") OCTSTR, 5, 1, 0, SKIP, 0, NULL},
	{FNAME("nonStandardAddress") SEQ, 0, 2, 2, SKIP, 0, NULL},
};

static const struct field_t _MulticastAddress_iPAddress[] = {	
	{FNAME("network") OCTSTR, FIXD, 4, 0, SKIP, 0, NULL},
	{FNAME("tsapIdentifier") INT, WORD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _MulticastAddress_iP6Address[] = {	
	{FNAME("network") OCTSTR, FIXD, 16, 0, SKIP, 0, NULL},
	{FNAME("tsapIdentifier") INT, WORD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _MulticastAddress[] = {	
	{FNAME("iPAddress") SEQ, 0, 2, 2, SKIP | EXT, 0,
	 _MulticastAddress_iPAddress},
	{FNAME("iP6Address") SEQ, 0, 2, 2, SKIP | EXT, 0,
	 _MulticastAddress_iP6Address},
	{FNAME("nsap") OCTSTR, 5, 1, 0, SKIP, 0, NULL},
	{FNAME("nonStandardAddress") SEQ, 0, 2, 2, SKIP, 0, NULL},
};

static const struct field_t _H245_TransportAddress[] = {	
	{FNAME("unicastAddress") CHOICE, 3, 5, 7, DECODE | EXT,
	 offsetof(H245_TransportAddress, unicastAddress), _UnicastAddress},
	{FNAME("multicastAddress") CHOICE, 1, 2, 4, SKIP | EXT, 0,
	 _MulticastAddress},
};

static const struct field_t _H2250LogicalChannelParameters[] = {	
	{FNAME("nonStandard") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _H2250LogicalChannelParameters_nonStandard},
	{FNAME("sessionID") INT, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("associatedSessionID") INT, 8, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("mediaChannel") CHOICE, 1, 2, 2, DECODE | EXT | OPT,
	 offsetof(H2250LogicalChannelParameters, mediaChannel),
	 _H245_TransportAddress},
	{FNAME("mediaGuaranteedDelivery") BOOL, FIXD, 0, 0, SKIP | OPT, 0,
	 NULL},
	{FNAME("mediaControlChannel") CHOICE, 1, 2, 2, DECODE | EXT | OPT,
	 offsetof(H2250LogicalChannelParameters, mediaControlChannel),
	 _H245_TransportAddress},
	{FNAME("mediaControlGuaranteedDelivery") BOOL, FIXD, 0, 0, STOP | OPT,
	 0, NULL},
	{FNAME("silenceSuppression") BOOL, FIXD, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("destination") SEQ, 0, 2, 2, STOP | EXT | OPT, 0, NULL},
	{FNAME("dynamicRTPPayloadType") INT, 5, 96, 0, STOP | OPT, 0, NULL},
	{FNAME("mediaPacketization") CHOICE, 0, 1, 2, STOP | EXT | OPT, 0,
	 NULL},
	{FNAME("transportCapability") SEQ, 3, 3, 3, STOP | EXT | OPT, 0,
	 NULL},
	{FNAME("redundancyEncoding") SEQ, 1, 2, 2, STOP | EXT | OPT, 0, NULL},
	{FNAME("source") SEQ, 0, 2, 2, SKIP | EXT | OPT, 0, NULL},
};

static const struct field_t _OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters[] = {	
	{FNAME("h222LogicalChannelParameters") SEQ, 3, 5, 5, SKIP | EXT, 0,
	 _H222LogicalChannelParameters},
	{FNAME("h223LogicalChannelParameters") SEQ, 0, 2, 2, SKIP | EXT, 0,
	 _H223LogicalChannelParameters},
	{FNAME("v76LogicalChannelParameters") SEQ, 0, 5, 5, SKIP | EXT, 0,
	 _V76LogicalChannelParameters},
	{FNAME("h2250LogicalChannelParameters") SEQ, 10, 11, 14, DECODE | EXT,
	 offsetof
	 (OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters,
	  h2250LogicalChannelParameters), _H2250LogicalChannelParameters},
	{FNAME("none") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _OpenLogicalChannel_forwardLogicalChannelParameters[] = {	
	{FNAME("portNumber") INT, WORD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("dataType") CHOICE, 3, 6, 9, DECODE | EXT,
	 offsetof(OpenLogicalChannel_forwardLogicalChannelParameters,
		  dataType), _DataType},
	{FNAME("multiplexParameters") CHOICE, 2, 3, 5, DECODE | EXT,
	 offsetof(OpenLogicalChannel_forwardLogicalChannelParameters,
		  multiplexParameters),
	 _OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters},
	{FNAME("forwardLogicalChannelDependency") INT, WORD, 1, 0, SKIP | OPT,
	 0, NULL},
	{FNAME("replacementFor") INT, WORD, 1, 0, SKIP | OPT, 0, NULL},
};

static const struct field_t _OpenLogicalChannel_reverseLogicalChannelParameters_multiplexParameters[] = {	
	{FNAME("h223LogicalChannelParameters") SEQ, 0, 2, 2, SKIP | EXT, 0,
	 _H223LogicalChannelParameters},
	{FNAME("v76LogicalChannelParameters") SEQ, 0, 5, 5, SKIP | EXT, 0,
	 _V76LogicalChannelParameters},
	{FNAME("h2250LogicalChannelParameters") SEQ, 10, 11, 14, DECODE | EXT,
	 offsetof
	 (OpenLogicalChannel_reverseLogicalChannelParameters_multiplexParameters,
	  h2250LogicalChannelParameters), _H2250LogicalChannelParameters},
};

static const struct field_t _OpenLogicalChannel_reverseLogicalChannelParameters[] = {	
	{FNAME("dataType") CHOICE, 3, 6, 9, SKIP | EXT, 0, _DataType},
	{FNAME("multiplexParameters") CHOICE, 1, 2, 3, DECODE | EXT | OPT,
	 offsetof(OpenLogicalChannel_reverseLogicalChannelParameters,
		  multiplexParameters),
	 _OpenLogicalChannel_reverseLogicalChannelParameters_multiplexParameters},
	{FNAME("reverseLogicalChannelDependency") INT, WORD, 1, 0, SKIP | OPT,
	 0, NULL},
	{FNAME("replacementFor") INT, WORD, 1, 0, SKIP | OPT, 0, NULL},
};

static const struct field_t _NetworkAccessParameters_distribution[] = {	
	{FNAME("unicast") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("multicast") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _Q2931Address_address[] = {	
	{FNAME("internationalNumber") NUMSTR, 4, 1, 0, SKIP, 0, NULL},
	{FNAME("nsapAddress") OCTSTR, 5, 1, 0, SKIP, 0, NULL},
};

static const struct field_t _Q2931Address[] = {	
	{FNAME("address") CHOICE, 1, 2, 2, SKIP | EXT, 0,
	 _Q2931Address_address},
	{FNAME("subaddress") OCTSTR, 5, 1, 0, SKIP | OPT, 0, NULL},
};

static const struct field_t _NetworkAccessParameters_networkAddress[] = {	
	{FNAME("q2931Address") SEQ, 1, 2, 2, SKIP | EXT, 0, _Q2931Address},
	{FNAME("e164Address") NUMDGT, 7, 1, 0, SKIP, 0, NULL},
	{FNAME("localAreaAddress") CHOICE, 1, 2, 2, DECODE | EXT,
	 offsetof(NetworkAccessParameters_networkAddress, localAreaAddress),
	 _H245_TransportAddress},
};

static const struct field_t _NetworkAccessParameters[] = {	
	{FNAME("distribution") CHOICE, 1, 2, 2, SKIP | EXT | OPT, 0,
	 _NetworkAccessParameters_distribution},
	{FNAME("networkAddress") CHOICE, 2, 3, 3, DECODE | EXT,
	 offsetof(NetworkAccessParameters, networkAddress),
	 _NetworkAccessParameters_networkAddress},
	{FNAME("associateConference") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("externalReference") OCTSTR, 8, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("t120SetupProcedure") CHOICE, 2, 3, 3, SKIP | EXT | OPT, 0,
	 NULL},
};

static const struct field_t _OpenLogicalChannel[] = {	
	{FNAME("forwardLogicalChannelNumber") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("forwardLogicalChannelParameters") SEQ, 1, 3, 5, DECODE | EXT,
	 offsetof(OpenLogicalChannel, forwardLogicalChannelParameters),
	 _OpenLogicalChannel_forwardLogicalChannelParameters},
	{FNAME("reverseLogicalChannelParameters") SEQ, 1, 2, 4,
	 DECODE | EXT | OPT, offsetof(OpenLogicalChannel,
				      reverseLogicalChannelParameters),
	 _OpenLogicalChannel_reverseLogicalChannelParameters},
	{FNAME("separateStack") SEQ, 2, 4, 5, DECODE | EXT | OPT,
	 offsetof(OpenLogicalChannel, separateStack),
	 _NetworkAccessParameters},
	{FNAME("encryptionSync") SEQ, 2, 4, 4, STOP | EXT | OPT, 0, NULL},
};

static const struct field_t _Setup_UUIE_fastStart[] = {	
	{FNAME("item") SEQ, 1, 3, 5, DECODE | OPEN | EXT,
	 sizeof(OpenLogicalChannel), _OpenLogicalChannel}
	,
};

static const struct field_t _Setup_UUIE[] = {	
	{FNAME("protocolIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("h245Address") CHOICE, 3, 7, 7, DECODE | EXT | OPT,
	 offsetof(Setup_UUIE, h245Address), _TransportAddress},
	{FNAME("sourceAddress") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _Setup_UUIE_sourceAddress},
	{FNAME("sourceInfo") SEQ, 6, 8, 10, SKIP | EXT, 0, _EndpointType},
	{FNAME("destinationAddress") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _Setup_UUIE_destinationAddress},
	{FNAME("destCallSignalAddress") CHOICE, 3, 7, 7, DECODE | EXT | OPT,
	 offsetof(Setup_UUIE, destCallSignalAddress), _TransportAddress},
	{FNAME("destExtraCallInfo") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _Setup_UUIE_destExtraCallInfo},
	{FNAME("destExtraCRV") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _Setup_UUIE_destExtraCRV},
	{FNAME("activeMC") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("conferenceID") OCTSTR, FIXD, 16, 0, SKIP, 0, NULL},
	{FNAME("conferenceGoal") CHOICE, 2, 3, 5, SKIP | EXT, 0,
	 _Setup_UUIE_conferenceGoal},
	{FNAME("callServices") SEQ, 0, 8, 8, SKIP | EXT | OPT, 0,
	 _QseriesOptions},
	{FNAME("callType") CHOICE, 2, 4, 4, SKIP | EXT, 0, _CallType},
	{FNAME("sourceCallSignalAddress") CHOICE, 3, 7, 7, DECODE | EXT | OPT,
	 offsetof(Setup_UUIE, sourceCallSignalAddress), _TransportAddress},
	{FNAME("remoteExtensionAddress") CHOICE, 1, 2, 7, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("callIdentifier") SEQ, 0, 1, 1, SKIP | EXT, 0, NULL},
	{FNAME("h245SecurityCapability") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("fastStart") SEQOF, SEMI, 0, 30, DECODE | OPT,
	 offsetof(Setup_UUIE, fastStart), _Setup_UUIE_fastStart},
	{FNAME("mediaWaitForConnect") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("canOverlapSend") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("endpointIdentifier") BMPSTR, 7, 1, 0, STOP | OPT, 0, NULL},
	{FNAME("multipleCalls") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("maintainConnection") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("connectionParameters") SEQ, 0, 3, 3, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("language") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("presentationIndicator") CHOICE, 2, 3, 3, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("screeningIndicator") ENUM, 2, 0, 0, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("serviceControl") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("symmetricOperationRequired") NUL, FIXD, 0, 0, SKIP | OPT, 0,
	 NULL},
	{FNAME("capacity") SEQ, 2, 2, 2, SKIP | EXT | OPT, 0, NULL},
	{FNAME("circuitInfo") SEQ, 3, 3, 3, SKIP | EXT | OPT, 0, NULL},
	{FNAME("desiredProtocols") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("neededFeatures") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("desiredFeatures") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("supportedFeatures") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("parallelH245Control") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("additionalSourceAddresses") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 NULL},
};

static const struct field_t _CallProceeding_UUIE_fastStart[] = {	
	{FNAME("item") SEQ, 1, 3, 5, DECODE | OPEN | EXT,
	 sizeof(OpenLogicalChannel), _OpenLogicalChannel}
	,
};

static const struct field_t _CallProceeding_UUIE[] = {	
	{FNAME("protocolIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("destinationInfo") SEQ, 6, 8, 10, SKIP | EXT, 0,
	 _EndpointType},
	{FNAME("h245Address") CHOICE, 3, 7, 7, DECODE | EXT | OPT,
	 offsetof(CallProceeding_UUIE, h245Address), _TransportAddress},
	{FNAME("callIdentifier") SEQ, 0, 1, 1, SKIP | EXT, 0, NULL},
	{FNAME("h245SecurityMode") CHOICE, 2, 4, 4, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("fastStart") SEQOF, SEMI, 0, 30, DECODE | OPT,
	 offsetof(CallProceeding_UUIE, fastStart),
	 _CallProceeding_UUIE_fastStart},
	{FNAME("multipleCalls") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("maintainConnection") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("fastConnectRefused") NUL, FIXD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, SKIP | EXT | OPT, 0, NULL},
};

static const struct field_t _Connect_UUIE_fastStart[] = {	
	{FNAME("item") SEQ, 1, 3, 5, DECODE | OPEN | EXT,
	 sizeof(OpenLogicalChannel), _OpenLogicalChannel}
	,
};

static const struct field_t _Connect_UUIE[] = {	
	{FNAME("protocolIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("h245Address") CHOICE, 3, 7, 7, DECODE | EXT | OPT,
	 offsetof(Connect_UUIE, h245Address), _TransportAddress},
	{FNAME("destinationInfo") SEQ, 6, 8, 10, SKIP | EXT, 0,
	 _EndpointType},
	{FNAME("conferenceID") OCTSTR, FIXD, 16, 0, SKIP, 0, NULL},
	{FNAME("callIdentifier") SEQ, 0, 1, 1, SKIP | EXT, 0, NULL},
	{FNAME("h245SecurityMode") CHOICE, 2, 4, 4, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("fastStart") SEQOF, SEMI, 0, 30, DECODE | OPT,
	 offsetof(Connect_UUIE, fastStart), _Connect_UUIE_fastStart},
	{FNAME("multipleCalls") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("maintainConnection") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("language") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("connectedAddress") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("presentationIndicator") CHOICE, 2, 3, 3, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("screeningIndicator") ENUM, 2, 0, 0, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("fastConnectRefused") NUL, FIXD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("serviceControl") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("capacity") SEQ, 2, 2, 2, SKIP | EXT | OPT, 0, NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, SKIP | EXT | OPT, 0, NULL},
};

static const struct field_t _Alerting_UUIE_fastStart[] = {	
	{FNAME("item") SEQ, 1, 3, 5, DECODE | OPEN | EXT,
	 sizeof(OpenLogicalChannel), _OpenLogicalChannel}
	,
};

static const struct field_t _Alerting_UUIE[] = {	
	{FNAME("protocolIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("destinationInfo") SEQ, 6, 8, 10, SKIP | EXT, 0,
	 _EndpointType},
	{FNAME("h245Address") CHOICE, 3, 7, 7, DECODE | EXT | OPT,
	 offsetof(Alerting_UUIE, h245Address), _TransportAddress},
	{FNAME("callIdentifier") SEQ, 0, 1, 1, SKIP | EXT, 0, NULL},
	{FNAME("h245SecurityMode") CHOICE, 2, 4, 4, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("fastStart") SEQOF, SEMI, 0, 30, DECODE | OPT,
	 offsetof(Alerting_UUIE, fastStart), _Alerting_UUIE_fastStart},
	{FNAME("multipleCalls") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("maintainConnection") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("alertingAddress") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("presentationIndicator") CHOICE, 2, 3, 3, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("screeningIndicator") ENUM, 2, 0, 0, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("fastConnectRefused") NUL, FIXD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("serviceControl") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("capacity") SEQ, 2, 2, 2, SKIP | EXT | OPT, 0, NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, SKIP | EXT | OPT, 0, NULL},
};

static const struct field_t _Information_UUIE[] = {	
	{FNAME("protocolIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("callIdentifier") SEQ, 0, 1, 1, SKIP | EXT, 0, NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("fastStart") SEQOF, SEMI, 0, 30, SKIP | OPT, 0, NULL},
	{FNAME("fastConnectRefused") NUL, FIXD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("circuitInfo") SEQ, 3, 3, 3, SKIP | EXT | OPT, 0, NULL},
};

static const struct field_t _ReleaseCompleteReason[] = {	
	{FNAME("noBandwidth") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("gatekeeperResources") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("unreachableDestination") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("destinationRejection") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("invalidRevision") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("noPermission") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("unreachableGatekeeper") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("gatewayResources") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("badFormatAddress") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("adaptiveBusy") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("inConf") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("undefinedReason") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("facilityCallDeflection") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("securityDenied") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("calledPartyNotRegistered") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("callerNotRegistered") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("newConnectionNeeded") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("nonStandardReason") SEQ, 0, 2, 2, SKIP, 0, NULL},
	{FNAME("replaceWithConferenceInvite") OCTSTR, FIXD, 16, 0, SKIP, 0,
	 NULL},
	{FNAME("genericDataReason") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("neededFeatureNotSupported") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("tunnelledSignallingRejected") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _ReleaseComplete_UUIE[] = {	
	{FNAME("protocolIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("reason") CHOICE, 4, 12, 22, SKIP | EXT | OPT, 0,
	 _ReleaseCompleteReason},
	{FNAME("callIdentifier") SEQ, 0, 1, 1, SKIP | EXT, 0, NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("busyAddress") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("presentationIndicator") CHOICE, 2, 3, 3, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("screeningIndicator") ENUM, 2, 0, 0, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("capacity") SEQ, 2, 2, 2, SKIP | EXT | OPT, 0, NULL},
	{FNAME("serviceControl") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, SKIP | EXT | OPT, 0, NULL},
};

static const struct field_t _Facility_UUIE_alternativeAliasAddress[] = {	
	{FNAME("item") CHOICE, 1, 2, 7, SKIP | EXT, 0, _AliasAddress},
};

static const struct field_t _FacilityReason[] = {	
	{FNAME("routeCallToGatekeeper") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("callForwarded") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("routeCallToMC") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("undefinedReason") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("conferenceListChoice") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("startH245") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("noH245") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("newTokens") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("featureSetUpdate") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("forwardedElements") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("transportedInformation") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _Facility_UUIE_fastStart[] = {	
	{FNAME("item") SEQ, 1, 3, 5, DECODE | OPEN | EXT,
	 sizeof(OpenLogicalChannel), _OpenLogicalChannel}
	,
};

static const struct field_t _Facility_UUIE[] = {	
	{FNAME("protocolIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("alternativeAddress") CHOICE, 3, 7, 7, DECODE | EXT | OPT,
	 offsetof(Facility_UUIE, alternativeAddress), _TransportAddress},
	{FNAME("alternativeAliasAddress") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _Facility_UUIE_alternativeAliasAddress},
	{FNAME("conferenceID") OCTSTR, FIXD, 16, 0, SKIP | OPT, 0, NULL},
	{FNAME("reason") CHOICE, 2, 4, 11, DECODE | EXT,
	 offsetof(Facility_UUIE, reason), _FacilityReason},
	{FNAME("callIdentifier") SEQ, 0, 1, 1, SKIP | EXT, 0, NULL},
	{FNAME("destExtraCallInfo") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("remoteExtensionAddress") CHOICE, 1, 2, 7, SKIP | EXT | OPT, 0,
	 NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("conferences") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("h245Address") CHOICE, 3, 7, 7, DECODE | EXT | OPT,
	 offsetof(Facility_UUIE, h245Address), _TransportAddress},
	{FNAME("fastStart") SEQOF, SEMI, 0, 30, DECODE | OPT,
	 offsetof(Facility_UUIE, fastStart), _Facility_UUIE_fastStart},
	{FNAME("multipleCalls") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("maintainConnection") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("fastConnectRefused") NUL, FIXD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("serviceControl") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("circuitInfo") SEQ, 3, 3, 3, SKIP | EXT | OPT, 0, NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, SKIP | EXT | OPT, 0, NULL},
	{FNAME("destinationInfo") SEQ, 6, 8, 10, SKIP | EXT | OPT, 0, NULL},
	{FNAME("h245SecurityMode") CHOICE, 2, 4, 4, SKIP | EXT | OPT, 0,
	 NULL},
};

static const struct field_t _CallIdentifier[] = {	
	{FNAME("guid") OCTSTR, FIXD, 16, 0, SKIP, 0, NULL},
};

static const struct field_t _SecurityServiceMode[] = {	
	{FNAME("nonStandard") SEQ, 0, 2, 2, SKIP, 0, _NonStandardParameter},
	{FNAME("none") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("default") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _SecurityCapabilities[] = {	
	{FNAME("nonStandard") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("encryption") CHOICE, 2, 3, 3, SKIP | EXT, 0,
	 _SecurityServiceMode},
	{FNAME("authenticaton") CHOICE, 2, 3, 3, SKIP | EXT, 0,
	 _SecurityServiceMode},
	{FNAME("integrity") CHOICE, 2, 3, 3, SKIP | EXT, 0,
	 _SecurityServiceMode},
};

static const struct field_t _H245Security[] = {	
	{FNAME("nonStandard") SEQ, 0, 2, 2, SKIP, 0, _NonStandardParameter},
	{FNAME("noSecurity") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("tls") SEQ, 1, 4, 4, SKIP | EXT, 0, _SecurityCapabilities},
	{FNAME("ipsec") SEQ, 1, 4, 4, SKIP | EXT, 0, _SecurityCapabilities},
};

static const struct field_t _DHset[] = {	
	{FNAME("halfkey") BITSTR, WORD, 0, 0, SKIP, 0, NULL},
	{FNAME("modSize") BITSTR, WORD, 0, 0, SKIP, 0, NULL},
	{FNAME("generator") BITSTR, WORD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _TypedCertificate[] = {	
	{FNAME("type") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("certificate") OCTSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _H235_NonStandardParameter[] = {	
	{FNAME("nonStandardIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("data") OCTSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _ClearToken[] = {	
	{FNAME("tokenOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("timeStamp") INT, CONS, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("password") BMPSTR, 7, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("dhkey") SEQ, 0, 3, 3, SKIP | EXT | OPT, 0, _DHset},
	{FNAME("challenge") OCTSTR, 7, 8, 0, SKIP | OPT, 0, NULL},
	{FNAME("random") INT, UNCO, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("certificate") SEQ, 0, 2, 2, SKIP | EXT | OPT, 0,
	 _TypedCertificate},
	{FNAME("generalID") BMPSTR, 7, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("nonStandard") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _H235_NonStandardParameter},
	{FNAME("eckasdhkey") CHOICE, 1, 2, 2, SKIP | EXT | OPT, 0, NULL},
	{FNAME("sendersID") BMPSTR, 7, 1, 0, SKIP | OPT, 0, NULL},
};

static const struct field_t _Progress_UUIE_tokens[] = {	
	{FNAME("item") SEQ, 8, 9, 11, SKIP | EXT, 0, _ClearToken},
};

static const struct field_t _Params[] = {	
	{FNAME("ranInt") INT, UNCO, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("iv8") OCTSTR, FIXD, 8, 0, SKIP | OPT, 0, NULL},
	{FNAME("iv16") OCTSTR, FIXD, 16, 0, SKIP | OPT, 0, NULL},
};

static const struct field_t _CryptoH323Token_cryptoEPPwdHash_token[] = {	
	{FNAME("algorithmOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("paramS") SEQ, 2, 2, 3, SKIP | EXT, 0, _Params},
	{FNAME("hash") BITSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _CryptoH323Token_cryptoEPPwdHash[] = {	
	{FNAME("alias") CHOICE, 1, 2, 7, SKIP | EXT, 0, _AliasAddress},
	{FNAME("timeStamp") INT, CONS, 1, 0, SKIP, 0, NULL},
	{FNAME("token") SEQ, 0, 3, 3, SKIP, 0,
	 _CryptoH323Token_cryptoEPPwdHash_token},
};

static const struct field_t _CryptoH323Token_cryptoGKPwdHash_token[] = {	
	{FNAME("algorithmOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("paramS") SEQ, 2, 2, 3, SKIP | EXT, 0, _Params},
	{FNAME("hash") BITSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _CryptoH323Token_cryptoGKPwdHash[] = {	
	{FNAME("gatekeeperId") BMPSTR, 7, 1, 0, SKIP, 0, NULL},
	{FNAME("timeStamp") INT, CONS, 1, 0, SKIP, 0, NULL},
	{FNAME("token") SEQ, 0, 3, 3, SKIP, 0,
	 _CryptoH323Token_cryptoGKPwdHash_token},
};

static const struct field_t _CryptoH323Token_cryptoEPPwdEncr[] = {	
	{FNAME("algorithmOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("paramS") SEQ, 2, 2, 3, SKIP | EXT, 0, _Params},
	{FNAME("encryptedData") OCTSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _CryptoH323Token_cryptoGKPwdEncr[] = {	
	{FNAME("algorithmOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("paramS") SEQ, 2, 2, 3, SKIP | EXT, 0, _Params},
	{FNAME("encryptedData") OCTSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _CryptoH323Token_cryptoEPCert[] = {	
	{FNAME("toBeSigned") SEQ, 8, 9, 11, SKIP | OPEN | EXT, 0, NULL},
	{FNAME("algorithmOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("paramS") SEQ, 2, 2, 3, SKIP | EXT, 0, _Params},
	{FNAME("signature") BITSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _CryptoH323Token_cryptoGKCert[] = {	
	{FNAME("toBeSigned") SEQ, 8, 9, 11, SKIP | OPEN | EXT, 0, NULL},
	{FNAME("algorithmOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("paramS") SEQ, 2, 2, 3, SKIP | EXT, 0, _Params},
	{FNAME("signature") BITSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _CryptoH323Token_cryptoFastStart[] = {	
	{FNAME("toBeSigned") SEQ, 8, 9, 11, SKIP | OPEN | EXT, 0, NULL},
	{FNAME("algorithmOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("paramS") SEQ, 2, 2, 3, SKIP | EXT, 0, _Params},
	{FNAME("signature") BITSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _CryptoToken_cryptoEncryptedToken_token[] = {	
	{FNAME("algorithmOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("paramS") SEQ, 2, 2, 3, SKIP | EXT, 0, _Params},
	{FNAME("encryptedData") OCTSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _CryptoToken_cryptoEncryptedToken[] = {	
	{FNAME("tokenOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("token") SEQ, 0, 3, 3, SKIP, 0,
	 _CryptoToken_cryptoEncryptedToken_token},
};

static const struct field_t _CryptoToken_cryptoSignedToken_token[] = {	
	{FNAME("toBeSigned") SEQ, 8, 9, 11, SKIP | OPEN | EXT, 0, NULL},
	{FNAME("algorithmOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("paramS") SEQ, 2, 2, 3, SKIP | EXT, 0, _Params},
	{FNAME("signature") BITSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _CryptoToken_cryptoSignedToken[] = {	
	{FNAME("tokenOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("token") SEQ, 0, 4, 4, SKIP, 0,
	 _CryptoToken_cryptoSignedToken_token},
};

static const struct field_t _CryptoToken_cryptoHashedToken_token[] = {	
	{FNAME("algorithmOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("paramS") SEQ, 2, 2, 3, SKIP | EXT, 0, _Params},
	{FNAME("hash") BITSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _CryptoToken_cryptoHashedToken[] = {	
	{FNAME("tokenOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("hashedVals") SEQ, 8, 9, 11, SKIP | EXT, 0, _ClearToken},
	{FNAME("token") SEQ, 0, 3, 3, SKIP, 0,
	 _CryptoToken_cryptoHashedToken_token},
};

static const struct field_t _CryptoToken_cryptoPwdEncr[] = {	
	{FNAME("algorithmOID") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("paramS") SEQ, 2, 2, 3, SKIP | EXT, 0, _Params},
	{FNAME("encryptedData") OCTSTR, SEMI, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _CryptoToken[] = {	
	{FNAME("cryptoEncryptedToken") SEQ, 0, 2, 2, SKIP, 0,
	 _CryptoToken_cryptoEncryptedToken},
	{FNAME("cryptoSignedToken") SEQ, 0, 2, 2, SKIP, 0,
	 _CryptoToken_cryptoSignedToken},
	{FNAME("cryptoHashedToken") SEQ, 0, 3, 3, SKIP, 0,
	 _CryptoToken_cryptoHashedToken},
	{FNAME("cryptoPwdEncr") SEQ, 0, 3, 3, SKIP, 0,
	 _CryptoToken_cryptoPwdEncr},
};

static const struct field_t _CryptoH323Token[] = {	
	{FNAME("cryptoEPPwdHash") SEQ, 0, 3, 3, SKIP, 0,
	 _CryptoH323Token_cryptoEPPwdHash},
	{FNAME("cryptoGKPwdHash") SEQ, 0, 3, 3, SKIP, 0,
	 _CryptoH323Token_cryptoGKPwdHash},
	{FNAME("cryptoEPPwdEncr") SEQ, 0, 3, 3, SKIP, 0,
	 _CryptoH323Token_cryptoEPPwdEncr},
	{FNAME("cryptoGKPwdEncr") SEQ, 0, 3, 3, SKIP, 0,
	 _CryptoH323Token_cryptoGKPwdEncr},
	{FNAME("cryptoEPCert") SEQ, 0, 4, 4, SKIP, 0,
	 _CryptoH323Token_cryptoEPCert},
	{FNAME("cryptoGKCert") SEQ, 0, 4, 4, SKIP, 0,
	 _CryptoH323Token_cryptoGKCert},
	{FNAME("cryptoFastStart") SEQ, 0, 4, 4, SKIP, 0,
	 _CryptoH323Token_cryptoFastStart},
	{FNAME("nestedcryptoToken") CHOICE, 2, 4, 4, SKIP | EXT, 0,
	 _CryptoToken},
};

static const struct field_t _Progress_UUIE_cryptoTokens[] = {	
	{FNAME("item") CHOICE, 3, 8, 8, SKIP | EXT, 0, _CryptoH323Token},
};

static const struct field_t _Progress_UUIE_fastStart[] = {	
	{FNAME("item") SEQ, 1, 3, 5, DECODE | OPEN | EXT,
	 sizeof(OpenLogicalChannel), _OpenLogicalChannel}
	,
};

static const struct field_t _Progress_UUIE[] = {	
	{FNAME("protocolIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("destinationInfo") SEQ, 6, 8, 10, SKIP | EXT, 0,
	 _EndpointType},
	{FNAME("h245Address") CHOICE, 3, 7, 7, DECODE | EXT | OPT,
	 offsetof(Progress_UUIE, h245Address), _TransportAddress},
	{FNAME("callIdentifier") SEQ, 0, 1, 1, SKIP | EXT, 0,
	 _CallIdentifier},
	{FNAME("h245SecurityMode") CHOICE, 2, 4, 4, SKIP | EXT | OPT, 0,
	 _H245Security},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _Progress_UUIE_tokens},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _Progress_UUIE_cryptoTokens},
	{FNAME("fastStart") SEQOF, SEMI, 0, 30, DECODE | OPT,
	 offsetof(Progress_UUIE, fastStart), _Progress_UUIE_fastStart},
	{FNAME("multipleCalls") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("maintainConnection") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("fastConnectRefused") NUL, FIXD, 0, 0, SKIP | OPT, 0, NULL},
};

static const struct field_t _H323_UU_PDU_h323_message_body[] = {	
	{FNAME("setup") SEQ, 7, 13, 39, DECODE | EXT,
	 offsetof(H323_UU_PDU_h323_message_body, setup), _Setup_UUIE},
	{FNAME("callProceeding") SEQ, 1, 3, 12, DECODE | EXT,
	 offsetof(H323_UU_PDU_h323_message_body, callProceeding),
	 _CallProceeding_UUIE},
	{FNAME("connect") SEQ, 1, 4, 19, DECODE | EXT,
	 offsetof(H323_UU_PDU_h323_message_body, connect), _Connect_UUIE},
	{FNAME("alerting") SEQ, 1, 3, 17, DECODE | EXT,
	 offsetof(H323_UU_PDU_h323_message_body, alerting), _Alerting_UUIE},
	{FNAME("information") SEQ, 0, 1, 7, SKIP | EXT, 0, _Information_UUIE},
	{FNAME("releaseComplete") SEQ, 1, 2, 11, SKIP | EXT, 0,
	 _ReleaseComplete_UUIE},
	{FNAME("facility") SEQ, 3, 5, 21, DECODE | EXT,
	 offsetof(H323_UU_PDU_h323_message_body, facility), _Facility_UUIE},
	{FNAME("progress") SEQ, 5, 8, 11, DECODE | EXT,
	 offsetof(H323_UU_PDU_h323_message_body, progress), _Progress_UUIE},
	{FNAME("empty") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("status") SEQ, 2, 4, 4, SKIP | EXT, 0, NULL},
	{FNAME("statusInquiry") SEQ, 2, 4, 4, SKIP | EXT, 0, NULL},
	{FNAME("setupAcknowledge") SEQ, 2, 4, 4, SKIP | EXT, 0, NULL},
	{FNAME("notify") SEQ, 2, 4, 4, SKIP | EXT, 0, NULL},
};

static const struct field_t _RequestMessage[] = {	
	{FNAME("nonStandard") SEQ, 0, 1, 1, STOP | EXT, 0, NULL},
	{FNAME("masterSlaveDetermination") SEQ, 0, 2, 2, STOP | EXT, 0, NULL},
	{FNAME("terminalCapabilitySet") SEQ, 3, 5, 5, STOP | EXT, 0, NULL},
	{FNAME("openLogicalChannel") SEQ, 1, 3, 5, DECODE | EXT,
	 offsetof(RequestMessage, openLogicalChannel), _OpenLogicalChannel},
	{FNAME("closeLogicalChannel") SEQ, 0, 2, 3, STOP | EXT, 0, NULL},
	{FNAME("requestChannelClose") SEQ, 0, 1, 3, STOP | EXT, 0, NULL},
	{FNAME("multiplexEntrySend") SEQ, 0, 2, 2, STOP | EXT, 0, NULL},
	{FNAME("requestMultiplexEntry") SEQ, 0, 1, 1, STOP | EXT, 0, NULL},
	{FNAME("requestMode") SEQ, 0, 2, 2, STOP | EXT, 0, NULL},
	{FNAME("roundTripDelayRequest") SEQ, 0, 1, 1, STOP | EXT, 0, NULL},
	{FNAME("maintenanceLoopRequest") SEQ, 0, 1, 1, STOP | EXT, 0, NULL},
	{FNAME("communicationModeRequest") SEQ, 0, 0, 0, STOP | EXT, 0, NULL},
	{FNAME("conferenceRequest") CHOICE, 3, 8, 16, STOP | EXT, 0, NULL},
	{FNAME("multilinkRequest") CHOICE, 3, 5, 5, STOP | EXT, 0, NULL},
	{FNAME("logicalChannelRateRequest") SEQ, 0, 3, 3, STOP | EXT, 0,
	 NULL},
};

static const struct field_t _OpenLogicalChannelAck_reverseLogicalChannelParameters_multiplexParameters[] = {	
	{FNAME("h222LogicalChannelParameters") SEQ, 3, 5, 5, SKIP | EXT, 0,
	 _H222LogicalChannelParameters},
	{FNAME("h2250LogicalChannelParameters") SEQ, 10, 11, 14, DECODE | EXT,
	 offsetof
	 (OpenLogicalChannelAck_reverseLogicalChannelParameters_multiplexParameters,
	  h2250LogicalChannelParameters), _H2250LogicalChannelParameters},
};

static const struct field_t _OpenLogicalChannelAck_reverseLogicalChannelParameters[] = {	
	{FNAME("reverseLogicalChannelNumber") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("portNumber") INT, WORD, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("multiplexParameters") CHOICE, 0, 1, 2, DECODE | EXT | OPT,
	 offsetof(OpenLogicalChannelAck_reverseLogicalChannelParameters,
		  multiplexParameters),
	 _OpenLogicalChannelAck_reverseLogicalChannelParameters_multiplexParameters},
	{FNAME("replacementFor") INT, WORD, 1, 0, SKIP | OPT, 0, NULL},
};

static const struct field_t _H2250LogicalChannelAckParameters_nonStandard[] = {	
	{FNAME("item") SEQ, 0, 2, 2, SKIP, 0, _H245_NonStandardParameter},
};

static const struct field_t _H2250LogicalChannelAckParameters[] = {	
	{FNAME("nonStandard") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _H2250LogicalChannelAckParameters_nonStandard},
	{FNAME("sessionID") INT, 8, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("mediaChannel") CHOICE, 1, 2, 2, DECODE | EXT | OPT,
	 offsetof(H2250LogicalChannelAckParameters, mediaChannel),
	 _H245_TransportAddress},
	{FNAME("mediaControlChannel") CHOICE, 1, 2, 2, DECODE | EXT | OPT,
	 offsetof(H2250LogicalChannelAckParameters, mediaControlChannel),
	 _H245_TransportAddress},
	{FNAME("dynamicRTPPayloadType") INT, 5, 96, 0, SKIP | OPT, 0, NULL},
	{FNAME("flowControlToZero") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("portNumber") INT, WORD, 0, 0, SKIP | OPT, 0, NULL},
};

static const struct field_t _OpenLogicalChannelAck_forwardMultiplexAckParameters[] = {	
	{FNAME("h2250LogicalChannelAckParameters") SEQ, 5, 5, 7, DECODE | EXT,
	 offsetof(OpenLogicalChannelAck_forwardMultiplexAckParameters,
		  h2250LogicalChannelAckParameters),
	 _H2250LogicalChannelAckParameters},
};

static const struct field_t _OpenLogicalChannelAck[] = {	
	{FNAME("forwardLogicalChannelNumber") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("reverseLogicalChannelParameters") SEQ, 2, 3, 4,
	 DECODE | EXT | OPT, offsetof(OpenLogicalChannelAck,
				      reverseLogicalChannelParameters),
	 _OpenLogicalChannelAck_reverseLogicalChannelParameters},
	{FNAME("separateStack") SEQ, 2, 4, 5, DECODE | EXT | OPT,
	 offsetof(OpenLogicalChannelAck, separateStack),
	 _NetworkAccessParameters},
	{FNAME("forwardMultiplexAckParameters") CHOICE, 0, 1, 1,
	 DECODE | EXT | OPT, offsetof(OpenLogicalChannelAck,
				      forwardMultiplexAckParameters),
	 _OpenLogicalChannelAck_forwardMultiplexAckParameters},
	{FNAME("encryptionSync") SEQ, 2, 4, 4, STOP | EXT | OPT, 0, NULL},
};

static const struct field_t _ResponseMessage[] = {	
	{FNAME("nonStandard") SEQ, 0, 1, 1, STOP | EXT, 0, NULL},
	{FNAME("masterSlaveDeterminationAck") SEQ, 0, 1, 1, STOP | EXT, 0,
	 NULL},
	{FNAME("masterSlaveDeterminationReject") SEQ, 0, 1, 1, STOP | EXT, 0,
	 NULL},
	{FNAME("terminalCapabilitySetAck") SEQ, 0, 1, 1, STOP | EXT, 0, NULL},
	{FNAME("terminalCapabilitySetReject") SEQ, 0, 2, 2, STOP | EXT, 0,
	 NULL},
	{FNAME("openLogicalChannelAck") SEQ, 1, 2, 5, DECODE | EXT,
	 offsetof(ResponseMessage, openLogicalChannelAck),
	 _OpenLogicalChannelAck},
	{FNAME("openLogicalChannelReject") SEQ, 0, 2, 2, STOP | EXT, 0, NULL},
	{FNAME("closeLogicalChannelAck") SEQ, 0, 1, 1, STOP | EXT, 0, NULL},
	{FNAME("requestChannelCloseAck") SEQ, 0, 1, 1, STOP | EXT, 0, NULL},
	{FNAME("requestChannelCloseReject") SEQ, 0, 2, 2, STOP | EXT, 0,
	 NULL},
	{FNAME("multiplexEntrySendAck") SEQ, 0, 2, 2, STOP | EXT, 0, NULL},
	{FNAME("multiplexEntrySendReject") SEQ, 0, 2, 2, STOP | EXT, 0, NULL},
	{FNAME("requestMultiplexEntryAck") SEQ, 0, 1, 1, STOP | EXT, 0, NULL},
	{FNAME("requestMultiplexEntryReject") SEQ, 0, 2, 2, STOP | EXT, 0,
	 NULL},
	{FNAME("requestModeAck") SEQ, 0, 2, 2, STOP | EXT, 0, NULL},
	{FNAME("requestModeReject") SEQ, 0, 2, 2, STOP | EXT, 0, NULL},
	{FNAME("roundTripDelayResponse") SEQ, 0, 1, 1, STOP | EXT, 0, NULL},
	{FNAME("maintenanceLoopAck") SEQ, 0, 1, 1, STOP | EXT, 0, NULL},
	{FNAME("maintenanceLoopReject") SEQ, 0, 2, 2, STOP | EXT, 0, NULL},
	{FNAME("communicationModeResponse") CHOICE, 0, 1, 1, STOP | EXT, 0,
	 NULL},
	{FNAME("conferenceResponse") CHOICE, 3, 8, 16, STOP | EXT, 0, NULL},
	{FNAME("multilinkResponse") CHOICE, 3, 5, 5, STOP | EXT, 0, NULL},
	{FNAME("logicalChannelRateAcknowledge") SEQ, 0, 3, 3, STOP | EXT, 0,
	 NULL},
	{FNAME("logicalChannelRateReject") SEQ, 1, 4, 4, STOP | EXT, 0, NULL},
};

static const struct field_t _MultimediaSystemControlMessage[] = {	
	{FNAME("request") CHOICE, 4, 11, 15, DECODE | EXT,
	 offsetof(MultimediaSystemControlMessage, request), _RequestMessage},
	{FNAME("response") CHOICE, 5, 19, 24, DECODE | EXT,
	 offsetof(MultimediaSystemControlMessage, response),
	 _ResponseMessage},
	{FNAME("command") CHOICE, 3, 7, 12, STOP | EXT, 0, NULL},
	{FNAME("indication") CHOICE, 4, 14, 23, STOP | EXT, 0, NULL},
};

static const struct field_t _H323_UU_PDU_h245Control[] = {	
	{FNAME("item") CHOICE, 2, 4, 4, DECODE | OPEN | EXT,
	 sizeof(MultimediaSystemControlMessage),
	 _MultimediaSystemControlMessage}
	,
};

static const struct field_t _H323_UU_PDU[] = {	
	{FNAME("h323-message-body") CHOICE, 3, 7, 13, DECODE | EXT,
	 offsetof(H323_UU_PDU, h323_message_body),
	 _H323_UU_PDU_h323_message_body},
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("h4501SupplementaryService") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 NULL},
	{FNAME("h245Tunneling") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("h245Control") SEQOF, SEMI, 0, 4, DECODE | OPT,
	 offsetof(H323_UU_PDU, h245Control), _H323_UU_PDU_h245Control},
	{FNAME("nonStandardControl") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("callLinkage") SEQ, 2, 2, 2, STOP | EXT | OPT, 0, NULL},
	{FNAME("tunnelledSignallingMessage") SEQ, 2, 4, 4, STOP | EXT | OPT,
	 0, NULL},
	{FNAME("provisionalRespToH245Tunneling") NUL, FIXD, 0, 0, STOP | OPT,
	 0, NULL},
	{FNAME("stimulusControl") SEQ, 3, 3, 3, STOP | EXT | OPT, 0, NULL},
	{FNAME("genericData") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
};

static const struct field_t _H323_UserInformation[] = {	
	{FNAME("h323-uu-pdu") SEQ, 1, 2, 11, DECODE | EXT,
	 offsetof(H323_UserInformation, h323_uu_pdu), _H323_UU_PDU},
	{FNAME("user-data") SEQ, 0, 2, 2, STOP | EXT | OPT, 0, NULL},
};

static const struct field_t _GatekeeperRequest[] = {	
	{FNAME("requestSeqNum") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("protocolIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("rasAddress") CHOICE, 3, 7, 7, DECODE | EXT,
	 offsetof(GatekeeperRequest, rasAddress), _TransportAddress},
	{FNAME("endpointType") SEQ, 6, 8, 10, STOP | EXT, 0, NULL},
	{FNAME("gatekeeperIdentifier") BMPSTR, 7, 1, 0, STOP | OPT, 0, NULL},
	{FNAME("callServices") SEQ, 0, 8, 8, STOP | EXT | OPT, 0, NULL},
	{FNAME("endpointAlias") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("alternateEndpoints") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("authenticationCapability") SEQOF, SEMI, 0, 0, STOP | OPT, 0,
	 NULL},
	{FNAME("algorithmOIDs") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("integrity") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("integrityCheckValue") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("supportsAltGK") NUL, FIXD, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, STOP | EXT | OPT, 0, NULL},
	{FNAME("genericData") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
};

static const struct field_t _GatekeeperConfirm[] = {	
	{FNAME("requestSeqNum") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("protocolIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("gatekeeperIdentifier") BMPSTR, 7, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("rasAddress") CHOICE, 3, 7, 7, DECODE | EXT,
	 offsetof(GatekeeperConfirm, rasAddress), _TransportAddress},
	{FNAME("alternateGatekeeper") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("authenticationMode") CHOICE, 3, 7, 8, STOP | EXT | OPT, 0,
	 NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("algorithmOID") OID, BYTE, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("integrity") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("integrityCheckValue") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, STOP | EXT | OPT, 0, NULL},
	{FNAME("genericData") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
};

static const struct field_t _RegistrationRequest_callSignalAddress[] = {	
	{FNAME("item") CHOICE, 3, 7, 7, DECODE | EXT,
	 sizeof(TransportAddress), _TransportAddress}
	,
};

static const struct field_t _RegistrationRequest_rasAddress[] = {	
	{FNAME("item") CHOICE, 3, 7, 7, DECODE | EXT,
	 sizeof(TransportAddress), _TransportAddress}
	,
};

static const struct field_t _RegistrationRequest_terminalAlias[] = {	
	{FNAME("item") CHOICE, 1, 2, 7, SKIP | EXT, 0, _AliasAddress},
};

static const struct field_t _RegistrationRequest[] = {	
	{FNAME("requestSeqNum") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("protocolIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("discoveryComplete") BOOL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("callSignalAddress") SEQOF, SEMI, 0, 10, DECODE,
	 offsetof(RegistrationRequest, callSignalAddress),
	 _RegistrationRequest_callSignalAddress},
	{FNAME("rasAddress") SEQOF, SEMI, 0, 10, DECODE,
	 offsetof(RegistrationRequest, rasAddress),
	 _RegistrationRequest_rasAddress},
	{FNAME("terminalType") SEQ, 6, 8, 10, SKIP | EXT, 0, _EndpointType},
	{FNAME("terminalAlias") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _RegistrationRequest_terminalAlias},
	{FNAME("gatekeeperIdentifier") BMPSTR, 7, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("endpointVendor") SEQ, 2, 3, 3, SKIP | EXT, 0,
	 _VendorIdentifier},
	{FNAME("alternateEndpoints") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("timeToLive") INT, CONS, 1, 0, DECODE | OPT,
	 offsetof(RegistrationRequest, timeToLive), NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("integrityCheckValue") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("keepAlive") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("endpointIdentifier") BMPSTR, 7, 1, 0, STOP | OPT, 0, NULL},
	{FNAME("willSupplyUUIEs") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("maintainConnection") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("alternateTransportAddresses") SEQ, 1, 1, 1, STOP | EXT | OPT,
	 0, NULL},
	{FNAME("additiveRegistration") NUL, FIXD, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("terminalAliasPattern") SEQOF, SEMI, 0, 0, STOP | OPT, 0,
	 NULL},
	{FNAME("supportsAltGK") NUL, FIXD, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("usageReportingCapability") SEQ, 3, 4, 4, STOP | EXT | OPT, 0,
	 NULL},
	{FNAME("multipleCalls") BOOL, FIXD, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("supportedH248Packages") SEQOF, SEMI, 0, 0, STOP | OPT, 0,
	 NULL},
	{FNAME("callCreditCapability") SEQ, 2, 2, 2, STOP | EXT | OPT, 0,
	 NULL},
	{FNAME("capacityReportingCapability") SEQ, 0, 1, 1, STOP | EXT | OPT,
	 0, NULL},
	{FNAME("capacity") SEQ, 2, 2, 2, STOP | EXT | OPT, 0, NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, STOP | EXT | OPT, 0, NULL},
	{FNAME("genericData") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
};

static const struct field_t _RegistrationConfirm_callSignalAddress[] = {	
	{FNAME("item") CHOICE, 3, 7, 7, DECODE | EXT,
	 sizeof(TransportAddress), _TransportAddress}
	,
};

static const struct field_t _RegistrationConfirm_terminalAlias[] = {	
	{FNAME("item") CHOICE, 1, 2, 7, SKIP | EXT, 0, _AliasAddress},
};

static const struct field_t _RegistrationConfirm[] = {	
	{FNAME("requestSeqNum") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("protocolIdentifier") OID, BYTE, 0, 0, SKIP, 0, NULL},
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("callSignalAddress") SEQOF, SEMI, 0, 10, DECODE,
	 offsetof(RegistrationConfirm, callSignalAddress),
	 _RegistrationConfirm_callSignalAddress},
	{FNAME("terminalAlias") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _RegistrationConfirm_terminalAlias},
	{FNAME("gatekeeperIdentifier") BMPSTR, 7, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("endpointIdentifier") BMPSTR, 7, 1, 0, SKIP, 0, NULL},
	{FNAME("alternateGatekeeper") SEQOF, SEMI, 0, 0, SKIP | OPT, 0, NULL},
	{FNAME("timeToLive") INT, CONS, 1, 0, DECODE | OPT,
	 offsetof(RegistrationConfirm, timeToLive), NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("integrityCheckValue") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("willRespondToIRR") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("preGrantedARQ") SEQ, 0, 4, 8, STOP | EXT | OPT, 0, NULL},
	{FNAME("maintainConnection") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("serviceControl") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("supportsAdditiveRegistration") NUL, FIXD, 0, 0, STOP | OPT, 0,
	 NULL},
	{FNAME("terminalAliasPattern") SEQOF, SEMI, 0, 0, STOP | OPT, 0,
	 NULL},
	{FNAME("supportedPrefixes") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("usageSpec") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("featureServerAlias") CHOICE, 1, 2, 7, STOP | EXT | OPT, 0,
	 NULL},
	{FNAME("capacityReportingSpec") SEQ, 0, 1, 1, STOP | EXT | OPT, 0,
	 NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, STOP | EXT | OPT, 0, NULL},
	{FNAME("genericData") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
};

static const struct field_t _UnregistrationRequest_callSignalAddress[] = {	
	{FNAME("item") CHOICE, 3, 7, 7, DECODE | EXT,
	 sizeof(TransportAddress), _TransportAddress}
	,
};

static const struct field_t _UnregistrationRequest[] = {	
	{FNAME("requestSeqNum") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("callSignalAddress") SEQOF, SEMI, 0, 10, DECODE,
	 offsetof(UnregistrationRequest, callSignalAddress),
	 _UnregistrationRequest_callSignalAddress},
	{FNAME("endpointAlias") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("nonStandardData") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("endpointIdentifier") BMPSTR, 7, 1, 0, STOP | OPT, 0, NULL},
	{FNAME("alternateEndpoints") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("gatekeeperIdentifier") BMPSTR, 7, 1, 0, STOP | OPT, 0, NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("integrityCheckValue") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("reason") CHOICE, 2, 4, 5, STOP | EXT | OPT, 0, NULL},
	{FNAME("endpointAliasPattern") SEQOF, SEMI, 0, 0, STOP | OPT, 0,
	 NULL},
	{FNAME("supportedPrefixes") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("alternateGatekeeper") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("genericData") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
};

static const struct field_t _CallModel[] = {	
	{FNAME("direct") NUL, FIXD, 0, 0, SKIP, 0, NULL},
	{FNAME("gatekeeperRouted") NUL, FIXD, 0, 0, SKIP, 0, NULL},
};

static const struct field_t _AdmissionRequest_destinationInfo[] = {	
	{FNAME("item") CHOICE, 1, 2, 7, SKIP | EXT, 0, _AliasAddress},
};

static const struct field_t _AdmissionRequest_destExtraCallInfo[] = {	
	{FNAME("item") CHOICE, 1, 2, 7, SKIP | EXT, 0, _AliasAddress},
};

static const struct field_t _AdmissionRequest_srcInfo[] = {	
	{FNAME("item") CHOICE, 1, 2, 7, SKIP | EXT, 0, _AliasAddress},
};

static const struct field_t _AdmissionRequest[] = {	
	{FNAME("requestSeqNum") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("callType") CHOICE, 2, 4, 4, SKIP | EXT, 0, _CallType},
	{FNAME("callModel") CHOICE, 1, 2, 2, SKIP | EXT | OPT, 0, _CallModel},
	{FNAME("endpointIdentifier") BMPSTR, 7, 1, 0, SKIP, 0, NULL},
	{FNAME("destinationInfo") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _AdmissionRequest_destinationInfo},
	{FNAME("destCallSignalAddress") CHOICE, 3, 7, 7, DECODE | EXT | OPT,
	 offsetof(AdmissionRequest, destCallSignalAddress),
	 _TransportAddress},
	{FNAME("destExtraCallInfo") SEQOF, SEMI, 0, 0, SKIP | OPT, 0,
	 _AdmissionRequest_destExtraCallInfo},
	{FNAME("srcInfo") SEQOF, SEMI, 0, 0, SKIP, 0,
	 _AdmissionRequest_srcInfo},
	{FNAME("srcCallSignalAddress") CHOICE, 3, 7, 7, DECODE | EXT | OPT,
	 offsetof(AdmissionRequest, srcCallSignalAddress), _TransportAddress},
	{FNAME("bandWidth") INT, CONS, 0, 0, STOP, 0, NULL},
	{FNAME("callReferenceValue") INT, WORD, 0, 0, STOP, 0, NULL},
	{FNAME("nonStandardData") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("callServices") SEQ, 0, 8, 8, STOP | EXT | OPT, 0, NULL},
	{FNAME("conferenceID") OCTSTR, FIXD, 16, 0, STOP, 0, NULL},
	{FNAME("activeMC") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("answerCall") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("canMapAlias") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("callIdentifier") SEQ, 0, 1, 1, STOP | EXT, 0, NULL},
	{FNAME("srcAlternatives") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("destAlternatives") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("gatekeeperIdentifier") BMPSTR, 7, 1, 0, STOP | OPT, 0, NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("integrityCheckValue") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("transportQOS") CHOICE, 2, 3, 3, STOP | EXT | OPT, 0, NULL},
	{FNAME("willSupplyUUIEs") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("callLinkage") SEQ, 2, 2, 2, STOP | EXT | OPT, 0, NULL},
	{FNAME("gatewayDataRate") SEQ, 2, 3, 3, STOP | EXT | OPT, 0, NULL},
	{FNAME("capacity") SEQ, 2, 2, 2, STOP | EXT | OPT, 0, NULL},
	{FNAME("circuitInfo") SEQ, 3, 3, 3, STOP | EXT | OPT, 0, NULL},
	{FNAME("desiredProtocols") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("desiredTunnelledProtocol") SEQ, 1, 2, 2, STOP | EXT | OPT, 0,
	 NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, STOP | EXT | OPT, 0, NULL},
	{FNAME("genericData") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
};

static const struct field_t _AdmissionConfirm[] = {	
	{FNAME("requestSeqNum") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("bandWidth") INT, CONS, 0, 0, SKIP, 0, NULL},
	{FNAME("callModel") CHOICE, 1, 2, 2, SKIP | EXT, 0, _CallModel},
	{FNAME("destCallSignalAddress") CHOICE, 3, 7, 7, DECODE | EXT,
	 offsetof(AdmissionConfirm, destCallSignalAddress),
	 _TransportAddress},
	{FNAME("irrFrequency") INT, WORD, 1, 0, STOP | OPT, 0, NULL},
	{FNAME("nonStandardData") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("destinationInfo") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("destExtraCallInfo") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("destinationType") SEQ, 6, 8, 10, STOP | EXT | OPT, 0, NULL},
	{FNAME("remoteExtensionAddress") SEQOF, SEMI, 0, 0, STOP | OPT, 0,
	 NULL},
	{FNAME("alternateEndpoints") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("integrityCheckValue") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("transportQOS") CHOICE, 2, 3, 3, STOP | EXT | OPT, 0, NULL},
	{FNAME("willRespondToIRR") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("uuiesRequested") SEQ, 0, 9, 13, STOP | EXT, 0, NULL},
	{FNAME("language") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("alternateTransportAddresses") SEQ, 1, 1, 1, STOP | EXT | OPT,
	 0, NULL},
	{FNAME("useSpecifiedTransport") CHOICE, 1, 2, 2, STOP | EXT | OPT, 0,
	 NULL},
	{FNAME("circuitInfo") SEQ, 3, 3, 3, STOP | EXT | OPT, 0, NULL},
	{FNAME("usageSpec") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("supportedProtocols") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("serviceControl") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("multipleCalls") BOOL, FIXD, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, STOP | EXT | OPT, 0, NULL},
	{FNAME("genericData") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
};

static const struct field_t _LocationRequest_destinationInfo[] = {	
	{FNAME("item") CHOICE, 1, 2, 7, SKIP | EXT, 0, _AliasAddress},
};

static const struct field_t _LocationRequest[] = {	
	{FNAME("requestSeqNum") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("endpointIdentifier") BMPSTR, 7, 1, 0, SKIP | OPT, 0, NULL},
	{FNAME("destinationInfo") SEQOF, SEMI, 0, 0, SKIP, 0,
	 _LocationRequest_destinationInfo},
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("replyAddress") CHOICE, 3, 7, 7, DECODE | EXT,
	 offsetof(LocationRequest, replyAddress), _TransportAddress},
	{FNAME("sourceInfo") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("canMapAlias") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("gatekeeperIdentifier") BMPSTR, 7, 1, 0, STOP | OPT, 0, NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("integrityCheckValue") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("desiredProtocols") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("desiredTunnelledProtocol") SEQ, 1, 2, 2, STOP | EXT | OPT, 0,
	 NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, STOP | EXT | OPT, 0, NULL},
	{FNAME("genericData") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("hopCount") INT, 8, 1, 0, STOP | OPT, 0, NULL},
	{FNAME("circuitInfo") SEQ, 3, 3, 3, STOP | EXT | OPT, 0, NULL},
};

static const struct field_t _LocationConfirm[] = {	
	{FNAME("requestSeqNum") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("callSignalAddress") CHOICE, 3, 7, 7, DECODE | EXT,
	 offsetof(LocationConfirm, callSignalAddress), _TransportAddress},
	{FNAME("rasAddress") CHOICE, 3, 7, 7, DECODE | EXT,
	 offsetof(LocationConfirm, rasAddress), _TransportAddress},
	{FNAME("nonStandardData") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("destinationInfo") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("destExtraCallInfo") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("destinationType") SEQ, 6, 8, 10, STOP | EXT | OPT, 0, NULL},
	{FNAME("remoteExtensionAddress") SEQOF, SEMI, 0, 0, STOP | OPT, 0,
	 NULL},
	{FNAME("alternateEndpoints") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("integrityCheckValue") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("alternateTransportAddresses") SEQ, 1, 1, 1, STOP | EXT | OPT,
	 0, NULL},
	{FNAME("supportedProtocols") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("multipleCalls") BOOL, FIXD, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("featureSet") SEQ, 3, 4, 4, STOP | EXT | OPT, 0, NULL},
	{FNAME("genericData") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("circuitInfo") SEQ, 3, 3, 3, STOP | EXT | OPT, 0, NULL},
	{FNAME("serviceControl") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
};

static const struct field_t _InfoRequestResponse_callSignalAddress[] = {	
	{FNAME("item") CHOICE, 3, 7, 7, DECODE | EXT,
	 sizeof(TransportAddress), _TransportAddress}
	,
};

static const struct field_t _InfoRequestResponse[] = {	
	{FNAME("nonStandardData") SEQ, 0, 2, 2, SKIP | OPT, 0,
	 _NonStandardParameter},
	{FNAME("requestSeqNum") INT, WORD, 1, 0, SKIP, 0, NULL},
	{FNAME("endpointType") SEQ, 6, 8, 10, SKIP | EXT, 0, _EndpointType},
	{FNAME("endpointIdentifier") BMPSTR, 7, 1, 0, SKIP, 0, NULL},
	{FNAME("rasAddress") CHOICE, 3, 7, 7, DECODE | EXT,
	 offsetof(InfoRequestResponse, rasAddress), _TransportAddress},
	{FNAME("callSignalAddress") SEQOF, SEMI, 0, 10, DECODE,
	 offsetof(InfoRequestResponse, callSignalAddress),
	 _InfoRequestResponse_callSignalAddress},
	{FNAME("endpointAlias") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("perCallInfo") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("tokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("cryptoTokens") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
	{FNAME("integrityCheckValue") SEQ, 0, 2, 2, STOP | OPT, 0, NULL},
	{FNAME("needResponse") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("capacity") SEQ, 2, 2, 2, STOP | EXT | OPT, 0, NULL},
	{FNAME("irrStatus") CHOICE, 2, 4, 4, STOP | EXT | OPT, 0, NULL},
	{FNAME("unsolicited") BOOL, FIXD, 0, 0, STOP, 0, NULL},
	{FNAME("genericData") SEQOF, SEMI, 0, 0, STOP | OPT, 0, NULL},
};

static const struct field_t _RasMessage[] = {	
	{FNAME("gatekeeperRequest") SEQ, 4, 8, 18, DECODE | EXT,
	 offsetof(RasMessage, gatekeeperRequest), _GatekeeperRequest},
	{FNAME("gatekeeperConfirm") SEQ, 2, 5, 14, DECODE | EXT,
	 offsetof(RasMessage, gatekeeperConfirm), _GatekeeperConfirm},
	{FNAME("gatekeeperReject") SEQ, 2, 5, 11, STOP | EXT, 0, NULL},
	{FNAME("registrationRequest") SEQ, 3, 10, 31, DECODE | EXT,
	 offsetof(RasMessage, registrationRequest), _RegistrationRequest},
	{FNAME("registrationConfirm") SEQ, 3, 7, 24, DECODE | EXT,
	 offsetof(RasMessage, registrationConfirm), _RegistrationConfirm},
	{FNAME("registrationReject") SEQ, 2, 5, 11, STOP | EXT, 0, NULL},
	{FNAME("unregistrationRequest") SEQ, 3, 5, 15, DECODE | EXT,
	 offsetof(RasMessage, unregistrationRequest), _UnregistrationRequest},
	{FNAME("unregistrationConfirm") SEQ, 1, 2, 6, STOP | EXT, 0, NULL},
	{FNAME("unregistrationReject") SEQ, 1, 3, 8, STOP | EXT, 0, NULL},
	{FNAME("admissionRequest") SEQ, 7, 16, 34, DECODE | EXT,
	 offsetof(RasMessage, admissionRequest), _AdmissionRequest},
	{FNAME("admissionConfirm") SEQ, 2, 6, 27, DECODE | EXT,
	 offsetof(RasMessage, admissionConfirm), _AdmissionConfirm},
	{FNAME("admissionReject") SEQ, 1, 3, 11, STOP | EXT, 0, NULL},
	{FNAME("bandwidthRequest") SEQ, 2, 7, 18, STOP | EXT, 0, NULL},
	{FNAME("bandwidthConfirm") SEQ, 1, 3, 8, STOP | EXT, 0, NULL},
	{FNAME("bandwidthReject") SEQ, 1, 4, 9, STOP | EXT, 0, NULL},
	{FNAME("disengageRequest") SEQ, 1, 6, 19, STOP | EXT, 0, NULL},
	{FNAME("disengageConfirm") SEQ, 1, 2, 9, STOP | EXT, 0, NULL},
	{FNAME("disengageReject") SEQ, 1, 3, 8, STOP | EXT, 0, NULL},
	{FNAME("locationRequest") SEQ, 2, 5, 17, DECODE | EXT,
	 offsetof(RasMessage, locationRequest), _LocationRequest},
	{FNAME("locationConfirm") SEQ, 1, 4, 19, DECODE | EXT,
	 offsetof(RasMessage, locationConfirm), _LocationConfirm},
	{FNAME("locationReject") SEQ, 1, 3, 10, STOP | EXT, 0, NULL},
	{FNAME("infoRequest") SEQ, 2, 4, 15, STOP | EXT, 0, NULL},
	{FNAME("infoRequestResponse") SEQ, 3, 8, 16, DECODE | EXT,
	 offsetof(RasMessage, infoRequestResponse), _InfoRequestResponse},
	{FNAME("nonStandardMessage") SEQ, 0, 2, 7, STOP | EXT, 0, NULL},
	{FNAME("unknownMessageResponse") SEQ, 0, 1, 5, STOP | EXT, 0, NULL},
	{FNAME("requestInProgress") SEQ, 4, 6, 6, STOP | EXT, 0, NULL},
	{FNAME("resourcesAvailableIndicate") SEQ, 4, 9, 11, STOP | EXT, 0,
	 NULL},
	{FNAME("resourcesAvailableConfirm") SEQ, 4, 6, 7, STOP | EXT, 0,
	 NULL},
	{FNAME("infoRequestAck") SEQ, 4, 5, 5, STOP | EXT, 0, NULL},
	{FNAME("infoRequestNak") SEQ, 5, 7, 7, STOP | EXT, 0, NULL},
	{FNAME("serviceControlIndication") SEQ, 8, 10, 10, STOP | EXT, 0,
	 NULL},
	{FNAME("serviceControlResponse") SEQ, 7, 8, 8, STOP | EXT, 0, NULL},
};
