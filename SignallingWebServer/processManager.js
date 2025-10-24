const Path = require("path");
const FS = require("fs");
const Spawn = require('child_process').spawn;
var Kill = require('tree-kill');

class UnrealProcess{
    constructor(exe_path, global_config, close_cb){
        this._param = global_config.UnrealProcessArgument;
        //["-AudioMixer", "-PixelStreamingIP=127.0.0.1", "-PixelStreamingPort=8888", "-ForceRes", "-RenderOffScreen",  "-Unattended"];
        this._exe_path = exe_path;
        this._close_cb = close_cb;
        this._global_config = global_config;
        this.Create();
    }

    Create(){
        
        // resolved execute path for different platform
        {
            let suffix = {
                'Windows' : '.exe',
                'Linux' : '.sh'
            };
            this._exe_path += suffix[this._global_config.Platform || 'Windows'];
            this._exe_path = Path.resolve(this._exe_path);
        }
        this._exe = Spawn(this._exe_path, this._param, {cwd:"./"});
        var exe = this._exe;
        var that = this;
        exe.on("error", function (code){
            console.log(code);
            that.Terminate();
        });
        exe.on("close", function(code){
            console.log(code);
            that.Terminate();
        })
        exe.on('message', function(data){
            console.log("message" + data.toString());
        });
        if(this._global_config.ProcessLogToCmd){
            exe.stdout.on('data', function(data){
                console.log("stdout:" +data.toString());
            });
        }
        exe.stderr.on('error', function(data){
            console.log("stderr:" + data.toString());
        });
    }

    Terminate(cb){
        var that = this;
        if(this._exe){
            Kill(this._exe.pid, "SIGKILL", function(){
                if(cb) cb();
                if(that._close_cb){
                    that._close_cb();
                }
            });
            this._exe = null;
        }else{
            if(cb) {cb();}
            if(that._close_cb){
                that._close_cb();
            }
        }
    }
}

module.exports = class ProcessManger{
    constructor(global_config){
        this._exe_path = "";
        this._process = null;
        this._status = "NotStart";
        this._waitter = [];
        this._wss = [];
        this._global_config = global_config;
        this.InitExePath();
        this.InitCtrlC();
        this.AutoPreloadProcess();
    }

    InitCtrlC(){
        var that = this;
        process.on('SIGINT', function () {
            that._global_config.AutoPreloadProcess = false;
            that.ShutdownProcess();
            setTimeout(function(){
                process.exit();
            }, 1000);
        });
        //然而并没用
        process.on('beforeExit', function () {
            that.ShutdownProcess();
        });
    }

    InitExePath(){
        if(FS.existsSync(this._global_config.UnrealProcessPath)){
            this._exe_path = this._global_config.UnrealProcessPath;
        }else{
            console.log("文件不存在：" + this._global_config.UnrealProcessPath);
        }
        
        //var dir = "../../../";
        /*var dir = "../../Output/WindowsNoEditor";
        var files = FS.readdirSync(dir);
        if(files && files.length){
            for(var i in files){
                console.log(files[i]);
                if(files[i].endsWith(".exe")){
                    console.log("Find exe: " + files[i])
                    this._exe_path = Path.join(dir, files[i]);
                    break;
                }
            }
        }
        if(!this._exe_path){
            console.log("没有找到exe文件：" + Path.resolve(dir));
        }*/
    }

    AutoPreloadProcess(){
        var that = this;
        if(that._global_config.AutoPreloadProcess){
            that.CreateProcess(null);
        }
    }

    InitWS(ws){
        if(!ws) return;
        this._wss.push(ws);
        var that = this;
        function RemoveWS(ws){
            for(var i in that._wss){
                if(that._wss[i] == ws){
                    that._wss.splice(i, 1);
                    break;
                }
            }
            if(that._wss.length == 0){
                that.DestroyProcess();
            }
        }
        ws.on("close", function(){
            RemoveWS(ws);
        });
        ws.on("error", function(){
            RemoveWS(ws);
        })
    }

    AddClicentWS(ws){
        this.InitWS(ws);
    }

    CreateProcess(){
        if(!this._exe_path) {
            console.log("文件不存在。");
            return;
        };
        if(this._process) return;
        var that = this;
        this._status = "Starting";
        this._process = new UnrealProcess(this._exe_path, this._global_config, function(){
            that._process = null;
            that._status = "NotStart";
        });
    }

    DestroyProcess(){
        var that = this;
        setTimeout(function(){
            if(that._wss.length == 0){
                that.ShutdownProcess();
            }
        }, this._global_config.AutoDestroyProcessTime || 5000);
    }

    ShutdownProcess(){
        if(this._process){
            var that = this;
            this._process.Terminate(function(){
                that._process = null;
                setTimeout(function(){
                    that.AutoPreloadProcess();
                }, 500);
            });
        }
    }

    StreamerStarted(bool){
        console.log("Unreal Started.");
        for(var i in this._waitter){
            this._waitter[i].resolve(bool);
        }
        this._status = "Started";
    }

    async WaitStreamer(){
        var that = this;
        if(this._status != "Started"){
            return new Promise(function(resolve, reject){
                that._waitter.push({
                    resolve: resolve,
                    reject: reject
                });
            })
        }
        else{
            return true;
        }
    }
}


