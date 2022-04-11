y.to(31, { v: 30 });
x.to(160, { v: 30 });

coil.start();

var COMMON_SAMPLE_RATE = 96000
var COMMON_SCAN_STEP = 4

var params =  { sample_rate: 96000, sleep_period: 4, x: { v: 20 } }
    var target_y = -70;
    var target_x = 360;
    var start_x = x.get()
    var start_y = y.get()
    var scan_step = params.scan_step || COMMON_SCAN_STEP
    var line_count = Math.floor(Math.abs(target_y - start_y) / scan_step)
    var loop_line_count = line_count - (line_count & 1)

    if (start_y > target_y) scan_to = -scan_step
    params.x.sample_rate = params.x.sample_rate || params.sample_rate || COMMON_SAMPLE_RATE

    progressReporter.taskStarted(line_count)   
    for (var i = 0; i < loop_line_count / 2; i++) {
        params.x.line = y.get()
        x.scan_to(target_x, params.x)
        y.by(scan_step, params.y)
        progressReporter.taskProgressed()

        params.x.line = y.get()
        x.scan_to(start_x, params.x)
        y.by(scan_step, params.y)
        progressReporter.taskProgressed()
    }

    if (line_count != loop_line_count) {
        params.x.line = y.get()
        x.scan_to(target_x)
        progressReporter.taskProgressed()
    }

    progressReporter.taskFinished()

coil.stop();

//scan_rect(130, 400, { sample_rate: 96000, sleep_period: 4, x: { v: 20 } });
