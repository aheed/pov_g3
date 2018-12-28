'use strict';

import { NOF_SECTORS, NOF_LEDS } from './pov_geometry.js'

class Pov {
    constructor() {
        this.povdata = [];

        let nofItems = NOF_SECTORS * NOF_LEDS;
        for(let sector=0; sector<nofItems; ++sector) {

            for(let led=0; led<nofItems; ++led) {
                let initval = 0;
                if(led > 32) {
                    initval = led;
                }

                this.povdata[(sector * NOF_LEDS) + led] = initval;
            }
        }

        Object.seal(this);
    }
}


/**
 * Draws a POV display onto a canvas
 * @param {Element} canv 
 * @param {Pov} pov 
 */
function drawpov(canv, pov) {

    
}
