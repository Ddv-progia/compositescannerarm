
//p = autoScanParams
var p = {
  initialHalfPeriod : 1000
  workingHalfPeriod : 68000
  minimumLevel : 0.024
  mode : 'search'
  sampleRate : 4400

  xVelocity : 10
  xAcceleration : 50
  yVelocity : 10
  yAcceleration : 50

  xStartPoint : 0
  xEndPoint : 100
  yStartPoint : 0
  yEndPoint : 100
  yStep : 2
  shouldReturn : false
};

coil.start({initialHalfPeriod: p.initialHalfPeriod,
			workingHalfPeriod: p.workingHalfPeriod,
			minimumLevel: p.minimumLevel,
			mode: p.mode})

audioDataCollector.start(p.sampleRate)

x.to(p.xStartPoint,{v: p.xVelocity,a: p.xAcceleration})
y.to(p.yStartPoint,{v: p.yVelocity,a: p.yAcceleration})

sleep(2)
audioDataCollector.discard()


if(p.xBack){
	scan_rect_uni(p.xEndPoint,p.yEndPoint,{scan_step: p.yStep, sample_rate: p.sampleRate, 
												x:{v: p.xVelocity,a: p.xAcceleration}, 
												y:{v: p.yVelocity,a: p.yAcceleration}
												xback:{v: p.xVelocity*2}})
}
else{
	scan_rect(p.xEndPoint,p.yEndPoint,{scan_step: p.yStep, sample_rate: p.sampleRate, 
											x:{v: p.xVelocity,a: p.xAcceleration}, 
											y:{v: p.yVelocity,a: p.yAcceleration}})
}
coil.stop()

if(p.shouldReturn){
	x.to(p.xStartPoint,{v: p.xVelocity*2})
	y.to(p.yStartPoint,{v: p.yVelocity*2})
}