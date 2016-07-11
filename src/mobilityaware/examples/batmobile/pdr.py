import os.path

baseDir = "results/";
protocols = ["Batman", "BatMobileV1", "BatMobile", "BatMobile_CSV",  "AODV", "AODV_CSV", "OLSR"];

def computePDR(_file):
	pdr = 0
	TX = 0
	RX = 0
	with open(_file) as f:
		lines = f.readlines()
	
		for line in lines:
			data = line.split(",")
			if len(data)>3:
				TX += int(data[1])
				RX += int(data[2])
	
	if TX>0:
		return float(RX)/float(TX)
	else:
		return 0

for protocol in protocols:
	path = baseDir + protocol + "_total"
	#print path

	if os.path.isfile(path):
		pdr = computePDR(path)
		print protocol + " " + str(pdr)
