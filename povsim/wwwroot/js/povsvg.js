'use strict';

var arcElems;
var cachedNofSectors;
var cachedNofLeds;
var cachedSvgHeight;
var cachedSvgWidth;


/**
 * Draws a POV display onto a canvas
 * @param {Element} svgElem 
 * @param {Object} pov 
 */
function drawpovsvg(svgElem, pov) {

    //alert("halloooj");
    console.log("drawpovsvg");

    var bBox = svgElem.getBBox();
    console.log('XxY', bBox.x + 'x' + bBox.y);
    console.log('size', bBox.width + 'x' + bBox.height);


    // Todo: Check if the geometry matches the cached svg collection.
    //       Set arcElems to null if that is not the case.

    cachedSvgHeight = svgElem.getAttribute("height");
    cachedSvgWidth = svgElem.getAttribute("width");

    let xc = cachedSvgHeight / 2;
    let yc = cachedSvgWidth / 2;
    //let radius = cachedSvgHeight /2;

    arcElems = null; //TEMP!

    if(!arcElems)
    {
        console.log("drawpovsvg2");
        arcElems = [];

//        // Create svg top element
//        let topElem = document.createElement('svg');

        // Create arc elements
        

        let lastx;
        let lasty;

        let radius = 100; //TEMP, should be led radius
        let angle = Math.PI;
        let deltaAngle = Math.PI / 16; //TEMP

        lastx = xc + radius * Math.cos(angle - deltaAngle); //TEMP
        lasty = yc + radius * -Math.sin(angle - deltaAngle); //TEMP

        let sizeFactor = 0.07;

        deltaAngle = Math.PI * 2 / pov.nofSectors;

        for(let led=0; led < pov.nofLeds; led++)
        {
            let ledRadius = pov.povledRadius[led];
            if(ledRadius < 0)
            {
                //startAng += Math.PI; //Add 180 degrees
                ledRadius = -ledRadius;
            }
            ledRadius *= sizeFactor;

            angle = Math.PI;
            lastx = xc + ledRadius * Math.cos(angle);
            lasty = yc + ledRadius * -Math.sin(angle);

            for(let sector=0; sector < pov.nofSectors; sector++)
            {
                //let startAng = aLen * sector;
                angle -= deltaAngle;
                
                let ledIndex = sector * pov.nofLeds + led;
                let strokeColor = '#' + pov.ledColors[ledIndex].toString(16).padStart(6, '0');

                let x = xc + ledRadius * Math.cos(angle);
                let y = yc + ledRadius * -Math.sin(angle);

                let d = `M ${lastx} ${lasty} A ${ledRadius} ${ledRadius} 0 0 0 ${x} ${y}`;
                //console.log(" -- " + d);

                let newElem = document.createElementNS("http://www.w3.org/2000/svg", 'path');

                newElem.setAttribute("d", d);
                newElem.setAttribute("stroke", strokeColor);
                newElem.setAttribute("stroke-width", "3");

                svgElem.appendChild(newElem);
                arcElems.push(newElem);
                

                lastx = x;
                lasty = y;

            }
        }
    }
}

