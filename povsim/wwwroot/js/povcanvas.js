'use strict';




/**
 * Draws a POV display onto a canvas
 * @param {Element} canv 
 * @param {Object} pov 
 */
function drawpov(canv, pov) {

    /*alert("hallooo");*/

    var ctx = canv.getContext("2d");

    ctx.clearRect(0, 0, canv.width, canv.height);

    /*ctx.fillStyle = "#000000"; 
    ctx.fillRect(0, 0, 150, 75);

    ctx.strokeStyle = "#00FF00";
    ctx.beginPath();
    ctx.arc(95, 50, 50, 0, 2 * Math.PI);
    ctx.stroke();

    ctx.moveTo(0, 0);
    ctx.lineTo(100, 100);
    ctx.stroke();*/

    let xc = canv.width / 2;
    let yc = canv.height / 2;

    let maxAbsLedRadius = Math.max(Math.abs(Math.max(...pov.povledRadius)), Math.abs(Math.min(...pov.povledRadius)))

    let radiusScale = (canv.height / 2) / maxAbsLedRadius;
    let aLen = 2 * Math.PI / pov.nofSectors; //rad

    ctx.lineWidth = canv.width * 0.008; //0.010 ; //px
    //ctx.lineCap='butt'; 

    //console.log("pov.nofSectors: " + pov.nofSectors);
    //console.log("pov.nofLeds: " + pov.nofLeds);
    for(let led=0; led < pov.nofLeds; led++)
    {
        for(let sector=0; sector < pov.nofSectors; sector++)
        {
            //console.log(pov.povledRadius[led]);
            //console.log( pov.povledAngle[led]);
            let startAng = aLen * sector;
            let ledAng =  startAng - (pov.povledAngle[led] / 10000);
            
            let ledRadius = pov.povledRadius[led];
            if(ledRadius < 0)
            {
                ledAng += Math.PI; //Add 180 degrees
                ledRadius = -ledRadius;
            }

            let ledIndex = sector * pov.nofLeds + led;
            let strokeColor = '#' + pov.ledColors[ledIndex].toString(16).padStart(6, '0');
            ctx.strokeStyle = strokeColor;
            
            ctx.beginPath();
            ctx.arc(xc, yc, ledRadius * radiusScale, ledAng, ledAng + aLen);
            ctx.stroke();
        }
    }

}


