//y.to(0, { v: 30 })
//x.to(0, { v: 30 })

coil.start()
scan_rect(300, -20, { 
	sample_rate: 96000, 
          sleep_time : 4,
	scan_step: 4, 
	x: { v: 20 },
	//xback: { v: 40 }
})
coil.stop()
