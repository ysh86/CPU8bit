/* Quartus II Version 9.0 Build 235 06/17/2009 Service Pack 2 SJ Web Edition */
JedecChain;
	FileRevision(JESD32A);
	DefaultMfr(6E);

	P ActionCode(Cfg)
		Device PartName(EP1K10T100) Path("D:/FPGA/CPU8bit/") File("CPU8bit.sof") MfrSpec(OpMask(1));
	P ActionCode(Ign)
		Device PartName(EPC2) MfrSpec(OpMask(0) FullPath("D:/FPGA/CPU8bit/CPU8bit.pof"));

ChainEnd;

AlteraBegin;
	ChainType(JTAG);
AlteraEnd;
