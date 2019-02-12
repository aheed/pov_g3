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

    console.log("drawpovsvg");

    var bBox = svgElem.getBBox();
    console.log('XxY', bBox.x + 'x' + bBox.y);
    console.log('size', bBox.width + 'x' + bBox.height);


    // Check if the geometry matches the cached svg collection.
    // Set arcElems to null if that is not the case.
    if (cachedSvgHeight != svgElem.getAttribute("height")) {
        cachedSvgHeight = svgElem.getAttribute("height");
        arcElems = null;
    }

    if (cachedSvgWidth != svgElem.getAttribute("width")) {
        cachedSvgWidth = svgElem.getAttribute("width");
        arcElems = null;
    }

    if (cachedNofSectors != pov.nofSectors) {
        cachedNofSectors = pov.nofSectors;
        arcElems = null;
    }

    if (cachedNofLeds != pov.nofLeds) {
        cachedNofLeds = pov.nofLeds;
        arcElems = null;
    }

    let xc = cachedSvgHeight / 2;
    let yc = cachedSvgWidth / 2;


    if(!arcElems)
    {
        console.log("drawpovsvg2");

        // Clean up any old elements
        while (svgElem.firstChild) {
            svgElem.removeChild(svgElem.firstChild);
        }
        arcElems = [];


        // Create arc elements

        let lastx;
        let lasty;

        let radius = 100; //TEMP, should be led radius
        let angle = Math.PI;
        let deltaAngle = Math.PI / 16; //TEMP

        lastx = xc + radius * Math.cos(angle - deltaAngle); //TEMP
        lasty = yc + radius * -Math.sin(angle - deltaAngle); //TEMP

        //let sizeFactor = 0.07;
        let maxAbsLedRadius = Math.max(Math.abs(Math.max(...pov.povledRadius)), Math.abs(Math.min(...pov.povledRadius)))
        let radiusScale = (svgElem.getAttribute("height") / 2) / maxAbsLedRadius;
        console.log(svgElem.getAttribute("height"));
        console.log(radiusScale);

        let strokeWidth = svgElem.getAttribute("width") *0.010 ; //px

        deltaAngle = Math.PI * 2 / pov.nofSectors;

        for(let led=0; led < pov.nofLeds; led++)
        {
            let ledRadius = pov.povledRadius[led];
            if(ledRadius < 0)
            {
                //startAng += Math.PI; //Add 180 degrees
                ledRadius = -ledRadius;
            }
            ledRadius *= radiusScale;

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
                newElem.setAttribute("stroke-width", strokeWidth);

                svgElem.appendChild(newElem);
                arcElems.push(newElem);
                

                lastx = x;
                lasty = y;

            }
        }
    }
    else {
        let elemIndex = 0;
        for(let led=0; led < pov.nofLeds; led++)
        {
            for(let sector=0; sector < pov.nofSectors; sector++)
            {
                let ledIndex = sector * pov.nofLeds + led;
                let strokeColor = '#' + pov.ledColors[ledIndex].toString(16).padStart(6, '0');
                let arcElem = arcElems[elemIndex++];
                //arcElem.setAttribute("stroke", strokeColor);
                arcElem.setAttribute('style', 'stroke:' + strokeColor);
                
            }
        }

    }
}

