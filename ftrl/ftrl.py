#!/usr/bin/env python
#coding:utf-8

from math import exp,sqrt
from time import sleep

sgn = lambda x: 1 if x>1e-6 else -1 if x<-1e-6 else 0

class ftrl():
	def __init__(self):
		self.file_wzn_out = "data/test2/wzn_20140506"
		self.file_train_in = "data/20140506"
		self.file_wzn_in = "wzn.in"
		#parameter
		#如果r1=0,产生一个一致的迭代公式；如果r1>0，则产生比较好的稀疏解
		self.r1 = 0
		self.r2 = 1e-5
		self.a = 1e-2*1.0
		self.b = 1*1.0
		#model	key(feature):value([w,z,n])
		self.wzn = self.initWZN()
	
	def initWZN(self):
		#format：feature:w,z,n
		rst = {}
		try:
			fp_in = open(self.file_wzn_in,"r")
			for line in fp_in:
				feature, wzn = line.strip().split(":")
				rst[feature] = [float(x.strip()) for x in wzn.split(",")]
			fp_in.close()
		except IOError:
			print "wzn init error! Get model feature"
			fp_in = open("data/bias_model_weight")
			for line in fp_in:
				feature, value = line.strip().split()
				rst[feature] = [float(value), 0.0, 0.0]
			fp_in.close()
		return rst
	
	def outputWZN(self,n):
		fp_out = open(self.file_wzn_out + "_" + str(n),"w")
		for k in self.wzn:
			line = "%s:%f,%f,%f\n" %(k, self.wzn[k][0], self.wzn[k][1], self.wzn[k][2])
			fp_out.write(line)
	
	def main(self):
		fp_train = open(self.file_train_in,"r")
		line_cnt = 1
		for line in fp_train:
			line = line.strip().split()
			yt, train, v = int(line[0]), line[1:], {}
			for t in train:
				k,value= t.split(":")
				if(k!="timestamp"):
					v[k] = float(value)
			#mail loop
			self.line_loop(v, yt)
			#print "yt%d" % yt
			line_cnt += 1
			if line_cnt % 100000 == 0: 
				print "%d" % line_cnt
			if line_cnt % 5000000 == 0:
				self.outputWZN(line_cnt)
				print "output WZN"
		self.outputWZN(line_cnt)
	
	def line_loop(self,v,yt):
		#fufil self.wzn
		for f in v:
			self.wzn.setdefault(f, [0.0, 0.0, 0.0])
		#change: self.wzn
		for f in self.wzn:
				#if self.wzn[f][0]!=0:
				if self.wzn[f][1] <= self.r1:
					self.wzn[f][0] = 0.0		
				else:
					self.wzn[f][0] = -(self.wzn[f][1] - sgn(self.wzn[f][1])*self.r1)/\
						(  self.r2 + (self.b + sqrt(self.wzn[f][2]))/self.a  )
		#count pt
		vw = 0.0
		for f in v:
			vw += v[f] * self.wzn[f][0]
		pt = 1.0/( 1+exp(vw) )
		#travelsal
		for f in v:
				gi = (pt - yt) * v[f]
				oi = ( (sqrt(self.wzn[f][2] + gi**2)) - sqrt(self.wzn[f][2]) ) / self.a
				self.wzn[f][1] = self.wzn[f][1] + gi - oi*self.wzn[f][0]
				self.wzn[f][2] = self.wzn[f][2] + gi**2

if __name__=="__main__":
	bob = ftrl()
	bob.main()
