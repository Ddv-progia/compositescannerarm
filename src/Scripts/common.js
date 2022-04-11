//
// common.js - common code for scripts
//

function StepMotor(nativeMotor)
{
    this.nativeMotor = nativeMotor
    this.velocity = 20
    this.acceleration = 200
}

StepMotor.prototype.setup = function(ps)
{
    if (typeof ps != 'undefined') {
	if (typeof ps.v != 'undefined') this.velocity = ps.v
	if (typeof ps.a != 'undefined') this.acceleration = ps.a
	if (typeof ps.sample_rate != 'undefined') this.sample_rate = ps.sample_rate
    }
}

StepMotor.prototype.to = function(target, ps)
{
    this.setup(ps)
    this.nativeMotor.moveToTechnological(this.velocity, this.acceleration, this.acceleration, target)
}

StepMotor.prototype.by = function(target, ps)
{
    this.setup(ps)
    this.nativeMotor.moveRelative(this.velocity, this.acceleration, this.acceleration, target)
}

StepMotor.prototype.to_m = function(target, ps)
{
    this.setup(ps)
    this.nativeMotor.moveToMachine(this.velocity, this.acceleration, this.acceleration, target)
}

StepMotor.prototype.get = function()
{
    return this.nativeMotor.getTechnologicalCoordinate()
}

StepMotor.prototype.get_m = function()
{
    return this.nativeMotor.getMachineCoordinate()
}

StepMotor.prototype.scan_to = function(target, ps)
{
    this.setup(ps)
    audioDataCollector.start(this.sample_rate || COMMON_SAMPLE_RATE)
    var start = this.get()
    this.nativeMotor.moveToTechnological(this.velocity, this.acceleration, this.acceleration, target)
    audioDataCollector.stop(start, this.get(), ps.line || -1)
}

StepMotor.prototype.scan_by = function(target, ps)
{
    this.setup(ps)
    audioDataCollector.start(this.sample_rate || COMMON_SAMPLE_RATE)
    var start = this.get()
    this.nativeMotor.moveRelative(this.velocity, this.acceleration, this.acceleration, target)
    audioDataCollector.stop(start, this.get(), ps.line || -1)
}

var COMMON_SAMPLE_RATE = 44000
var COMMON_SCAN_STEP = 5

var x = new StepMotor(builtin_xAxisMotor)
var y = new StepMotor(builtin_yAxisMotor)

function scan_rect(target_x, target_y, params)
{  
    var start_x = x.get()
    var start_y = y.get()
    var scan_step = params.scan_step || COMMON_SCAN_STEP
    var line_count = Math.floor(Math.abs(target_y - start_y) / scan_step)
    var loop_line_count = line_count - (line_count & 1)

    if (start_y > target_y) scan_step = -scan_step
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
}

function scan_rect_uni(target_x, target_y, params)
{  
    var start_x = x.get()
    var start_y = y.get()
    var scan_step = params.scan_step || COMMON_SCAN_STEP
    var line_count = Math.floor(Math.abs(target_y - start_y) / scan_step)

    if (start_y > target_y) scan_step = -scan_step
    params.x.sample_rate = params.x.sample_rate || params.sample_rate || COMMON_SAMPLE_RATE

    progressReporter.taskStarted(line_count)
    for (var i = 0; i < line_count; i++) {
        params.x.line = y.get()
        x.scan_to(target_x, params.x)
        x.to(start_x, params.xback)
        y.by(scan_step, params.y)
        progressReporter.taskProgressed()
    }
    progressReporter.taskFinished()
}

function Coil(nativeCoil)
{
    this.nativeCoil = nativeCoil
}

Coil.prototype.start = function (params)
{
    if (typeof params != 'undefined') {
        if (typeof params.halfPeriod != 'undefined') this.nativeCoil.halfPeriod = params.halfPeriod
        if (typeof params.initialHalfPeriod != 'undefined') this.nativeCoil.initialHalfPeriod = params.initialHalfPeriod
        if (typeof params.workingHalfPeriod != 'undefined') this.nativeCoil.workingHalfPeriod = params.workingHalfPeriod
        if (typeof params.minimumLevel != 'undefined') this.nativeCoil.minimumLevel = params.minimumLevel
    }

    if (typeof params != 'undefined') {
        switch (params.mode) {
        case 'dumb':
            this.nativeCoil.switchOnGenerator()
            break
        case 'single':
            this.nativeCoil.startSingle()
            break
        case 'search':
            this.nativeCoil.searchWorkingRange()
            break
        case 'normal':
        default:
            this.nativeCoil.start()
        }
    } else {
        this.nativeCoil.start()
    }
}

Coil.prototype.stop = function ()
{
    this.nativeCoil.stop()
}

coil = new Coil(builtin_coil)
