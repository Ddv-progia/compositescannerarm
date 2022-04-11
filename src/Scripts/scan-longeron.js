y.to(0, { v: 30 })
x.to(0, { v: 30 })

coil.start()
scan_rect(452, 18, { 
	sample_rate: 192000, 
	scan_step: 2, 
	x: { v: 20 },
	xback: { v: 40 }
})
coil.stop()
