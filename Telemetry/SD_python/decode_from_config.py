import sys
import json
import binascii
import os


db = {} #current "database" is only a dict, can be ported to actual database
filename = "extract"
folder_name = "extracted/"

#extract all the fields from the csv, writes data to both a dict (key is (id e.g 0xC2, Timestamp in mill))
def main():
	if not os.path.exists(folder_name):
		os.makedirs(folder_name)
	if len(sys.argv) != 2:
		print('Usage:')
		print('debug.py <raw xbee file>')
		quit()

	not_implemented = set()


	config_file = open('config.json')
	conf = json.load(config_file)
	f = open(sys.argv[1])
	c=0
	f.readline() #discard the header of the csv
	for key in conf.keys():

		file = open(folder_name+filename+conf.get(key).get("name")+".csv",'w+')
		s= 'Timestamp,'
		for i in range(len(conf.get(key).get("fields"))):
			s += conf.get(key).get("fields")[i].get("id") + ','
		s = s[:-1] + '\n'
		file.write(s)
		file.close()


	for line in f.readlines():
		c+=1
		tab = line.split(',') #0 timestamp 1 msg.id 2 f 3 val
		tab[3] = tab[3].rstrip('\n')
		tab[3] = tab[3].rstrip('\r')	
		if (conf.get(tab[1]) == None):
			not_implemented.add(tab[1])
			continue
		if tab[3].find('+') != -1:
			# convert scientifc into hex
			tab[3] = hex(int(float(tab[3])))
			
		db[tab[1],tab[0]] = {}
		parse_fields(tab,(conf.get(tab[1])["fields"]),conf.get(tab[1]).get("name"))
		

	print('not_implemented {}'.format(not_implemented))
	print('parsed sucessfully {} out of {} lines'.format(len(list(db.keys())),c))


def parse_fields(tab, fields, name):	
	file = open(folder_name+filename+name+".csv",'a+') #FIXME : opening file every time is extremelly inefficient, would be preferable to write to csv at the end from the dict/database
	x=bin(int(tab[3],16))[2:].zfill(64) #converts to bits - python is not the best choice to do this
	for f in fields:
		a = f.get("pos")
		b = f.get("size")
		if x[len(x) - a -b : len(x)-a] == "":
			print("val=%s a=%i b=%i"  %(val,a,b))
		db[tab[1],tab[0]][f.get("id")] = int(x[len(x) - a -b : len(x)-a],2)	
	s=str(tab[0]) + ','
	for key in db[tab[1],tab[0]].keys():
		s += str(db[tab[1],tab[0]].get(key)) + ','
	s = s[:-1] + '\n'
	file.write(s)
	file.close()



def dict_to_csv(filename):
	
	for key in db.keys():
		f.write('{},{},')	
		
main()