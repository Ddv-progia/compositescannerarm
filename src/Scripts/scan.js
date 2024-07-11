y.to(82, { v: 30 });
x.to(-426, { v: 30 });

coil.start();
scan_rect(20, 356, { sample_rate: 96000, scan_step: 10, sleep_period: 4, x: { v: 20 } });
coil.stop();
