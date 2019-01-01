"use strict";


var connection = new signalR.HubConnectionBuilder().withUrl("/povsim").build();
var latestPovData;

connection.on("ReceiveMessage", function (user, message) {
    var msg = message.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
    var encodedMsg = user + " says " + msg;
    var li = document.createElement("li");
    li.textContent = encodedMsg;
    document.getElementById("messagesList").appendChild(li);
});

/*connection.on("Test2", function (message) {
    var msg = message.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
    var encodedMsg = "got Test2: " + msg;
    var li = document.createElement("li");
    li.textContent = encodedMsg;
    document.getElementById("messagesList").appendChild(li);
});*/

connection.on("UpdatePOV", function (povd) {
    var encodedMsg = "got UpdatePOV " +
      povd.nofSectors.toString() + " " +
      povd.nofLeds.toString() + " " +
      povd.ledColors.length;
    /*var li = document.createElement("li");
    li.textContent = encodedMsg;
    document.getElementById("messagesList").appendChild(li);*/
    console.log(encodedMsg);

    let canvasElem = document.getElementById("myCanvas");
    latestPovData = povd;
    drawpov(canvasElem, povd);
});

connection.start().catch(function (err) {
    return console.error(err.toString());
});

/*
document.getElementById("sendButton").addEventListener("click", function (event) {
    var user = document.getElementById("userInput").value;
    var message = document.getElementById("messageInput").value;
    connection.invoke("SendMessage", user, message).catch(function (err) {
        return console.error(err.toString());
    });
    event.preventDefault();
});
*/

function resizeHandler(evt) {
    this.console.log("yo, window resized!!!");
    let canvasElem = document.getElementById("myCanvas");

    let evtObj = evt.srcElement || evt.currentTarget;
    canvasElem.width = Math.min(evtObj.innerWidth, evtObj.innerHeight);
    canvasElem.height = canvasElem.width; // keep it square
    this.console.log("w:" + canvasElem.width + " h:" + canvasElem.height);
    if(latestPovData) {
        drawpov(canvasElem, latestPovData);
    }
}

window.addEventListener('resize', resizeHandler);

// Trigger the window resize event once to set correct canvas size
window.dispatchEvent(new Event('resize'));

