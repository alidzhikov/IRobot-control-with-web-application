 url = "http://192.168.43.110";
var manager = "";
var wasJoystickControlSent = true;
var iRobotControlApp = new Vue({
    el: '#iRobotControlApp',
    data: {
        message: 'Hello Create!',
        irobotOn: false,
        irobotOperatingMode: '',
        irobotDemo: -1,
        powerLed: {color: 0, intensity: 0},
        playLed: false,
        advanceLed: false,
        speedLvl: 500,
        speedFixed: 2,
        songId: 0,
        translational: 0,
        angular: 0,
        angularLimit: 4.25,
        translationalLimit: 500
      },
    computed: {
        irobotMode: function () {
          return this.irobotOn ? this.getOperatingModeText() : 'Изключен';
        },
        irobotLeds: function() {
            let leds = 0;
            if(this.advanceLed){
                leds = leds+8;
            }
            if(this.playLed){
                leds = leds+2;
            }
            let color = 255/10 * (this.powerLed.color < 11 ? this.powerLed.color : 0 );
            let intensity = 255/10 * (this.powerLed.intensity < 11 ? this.powerLed.intensity : 0);
            axios.post(url + '/irobotLeds', {leds: [leds, color, intensity]}).then(res => {
                console.log(res);
            }).catch(err => {
                console.log(err);
            });
            return [leds, color, intensity];
        },
        getSensorData: function () {
            
        },
        getSpeedLevelFixed: function(){
            return this.speedFixed == 1;
        },
        getTranslational: function(){
            return this.translational;
        },
        getAngular: function(){
            return this.angular;
        },
        getMotorA: function(){
            return Number(this.translational) - Number(this.angular)*117.5;  //  -500+82 / 117.5
        },
        getMotorB: function(){
            return Number(this.translational) + Number(this.angular)*117.5;
        },
        getAngularLimit(){
            this.angularLimit = Math.round((500-Math.abs(Number(this.translational)))/117.5 * 100)/100;
            return  this.angularLimit ; 
        },
        getTranslationalLimit(){
            this.translationalLimit = Math.round((500-Math.abs(Number(this.angular)*117.5)) * 100)/100;
            return this.translationalLimit;
        }
    },
    methods: {
        getOperatingModeText: function () {
            return  this.irobotOperatingMode == 'passive' ? 'Пасивен' :
                this.irobotOperatingMode == 'safe' ? 'Безопасен' :
                this.irobotOperatingMode == 'full' ? 'Пълен' : 
                this.irobotOperatingMode == '' ? 'Не е зададен режим на работа' :
                this.irobotOperatingMode;
        },
        startDemo: function (demo) {
            this.irobotDemo = demo;
            axios.post(url + '/startDemo', { demo: demo })
            .then(response =>  console.log(response))
            .catch(err => console.log(err));
            this.irobotOperatingMode = 'passive';
        },
        playSong: function () {
            axios.post(url + '/playSong', { songId: this.songId })
            .then(response => console.log(response))
            .catch(err=> console.log(err));
        },
        driveiRobotWithJoystick: function(driveEvent) {
            let radius = Math.round(driveEvent.angle.degree);
            radius = radius >= 180 ? radius - 180 : radius;
            radius = radius >= 90 ? radius - 90 : radius;
            radius = (2000/90 * radius);
            if((driveEvent.angle.degree > 180 && driveEvent.angle.degree < 270) ||
                driveEvent.angle.degree < 90 ){
                    radius = 2000 - radius;
                }
                let x = driveEvent && driveEvent.direction && driveEvent.direction.x ? driveEvent.direction.x : ''; 
                let y = driveEvent && driveEvent.direction && driveEvent.direction.y ? driveEvent.direction.y : ''; 
            if((x == 'right' && y == 'up') || (x == 'left' && y == 'down')){
                radius = -radius;
            }

            let velocity = this.speedLvl;
            if(!this.speedFixed){
                velocity = this.speedLvl/75 * Math.round(driveEvent.distance);
            }
            if(driveEvent.direction.y == 'down'){
                velocity = -velocity;
            }
            radius = twosComplementBinary(radius);
            velocity = twosComplementBinary(velocity);
            if(wasJoystickControlSent){
                wasJoystickControlSent = false;
                axios.post(url + '/driveWithJoystick', {
                    radius: radius, 
                    velocity: velocity
                }, {
                    'Content-Type': 'application/x-www-form-urlencoded',
                })
                .then(response => {
                   console.log(response);
                   wasJoystickControlSent = true;
                })
                .catch(function(err) {
                   console.log(err);
                   wasJoystickControlSent = true;
                });
            }
        },
        driveXY: function(){
            let angularSpd = twosComplementBinary(-20 * Number(this.getAngular));
            let translationalSpd = twosComplementBinary(5 * Number(this.getTranslational));
         
            axios.post(url + '/driveWithJoystick', {
                radius: angularSpd, 
                velocity: translationalSpd
            }, {
                'Content-Type': 'application/x-www-form-urlencoded',
            })
            .then(response => { console.log(response); })
            .catch(function(err) { console.log(err); });
        },
        driveInASquare: function(){
            axios.post(url + '/driveInASquare', {
                'Content-Type': 'application/x-www-form-urlencoded',
            })
            .then(response => {
               console.log(response);             
            })
            .catch(function(err) {
               console.log(err);
            });
        },
        driveStop: function(){
            axios.post(url + '/driveStop',{
                'Content-Type': 'application/x-www-form-urlencoded',
            }).then(response => {
                console.log(response);             
            })
            .catch(function(err) {
               console.log(err);
            });
        },
        driveDiff: function(){
            axios.post(url + '/driveDiff', {
                motorA: twosComplementBinary(this.getMotorA), 
                motorB: twosComplementBinary(this.getMotorB)
            }, {
                'Content-Type': 'application/x-www-form-urlencoded',
            })
            .then(response => { console.log(response); })
            .catch(function(err) { console.log(err); });
        },
        getModeInBytes(mode){
            switch(mode){
                case "safe":
                return 131;
                case "full":
                return 132;
                default:
                return null;
            }
        },
        resetDriveVals: function(){
            this.angular = 0;
            this.translational = 0;
        }
    },
    watch: {
        irobotOperatingMode: function (newMode, oldQuestion) {
            if(this.getModeInBytes(newMode)){
                axios.post(url + '/operatingMode', { mode: this.getModeInBytes(newMode) }).then((response) => {
                    console.log(response);
                }).catch(err=> console.log(err));
            }

            if(this.irobotOperatingMode == 'safe' || this.irobotOperatingMode == 'full') {
                var driveRobotEvent = this.driveiRobotWithJoystick.bind(this);
                setTimeout(() => {
                    if(manager == ""){
                        var options = {
                                zone: document.getElementById('nipplejs-control'),
                                color: 'green',
                                size: '150',
                                position: {top: '550px', left: '20%'},
                                mode: 'static'
                            };
                        manager = nipplejs.create(options);
                        manager.on('move', function (evt, data) {
                            driveRobotEvent(data);
                        });
                    }
                });
            }else{
                //manager.destroy();
            }
        },
        irobotOn: function(isOn){
            if(isOn){
                axios.get(url + '/irobotOn').then((response) => {
                    this.irobotOperatingMode = 'passive';
                }).catch(err=> console.log(err));
            }else{
                this.irobotOperatingMode = '';
            }
        },
        powerLed:{
            handler: function (){
                this.irobotLeds;
            },
            deep:true
        },
        playLed: function (){
            this.irobotLeds;
        },
        advanceLed: function (){
            this.irobotLeds;
        }
      }
  });

  function twosComplementBinary (decimalNumber){
    decimalNumber = Math.ceil(decimalNumber);
    var strWithZeros = "0000000000000000";
    if(decimalNumber >= 0){
        var calculateComplement = decimalNumber.toString(2);
        if(calculateComplement.length < 16){
            calculateComplement = strWithZeros.slice(0,(16-calculateComplement.length) ) + calculateComplement;
        }
    }else{
        calculateComplement = (65536 + decimalNumber).toString(2);
    }
    var result = [ parseInt(calculateComplement.slice(0,8), 2),parseInt(calculateComplement.slice(8), 2) ];
    console.log(result)
    return result;
}
console.log(twosComplementBinary(200.3));
console.log(twosComplementBinary(-200.3));
console.log(twosComplementBinary(-0.3));
console.log(twosComplementBinary(0.5));
/*
let scores = [100, 90, 90, 80, 75, 60];
let alice = [50, 65, 77, 90, 102];
let aliceScores = [];
let functionReturnsa = alice.forEach(aliceScore => {  
	scores.push(aliceScore);
    scores.sort((a,b)=>b-a);
    let denseIndex = 1;

    scores.forEach((score,index) => {
        if(score == aliceScore && (index < scores.length || scores[index + 1] != score)){
            aliceScores.push(denseIndex);
        }
        if(index < scores.length && scores[index + 1] != score){
            denseIndex++;
        }

    });
});
return aliceScores;
*/