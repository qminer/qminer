
var socket = io();
var send = function () {
    var text = document.getElementById('text').value;
    var e = document.getElementById('RegularSpam');
    var value = e.options[e.selectedIndex].value;

    if (text != null) {
        socket.emit('InputText', {
            text: text,
            clasification: value
        });

        // adding a new text to the screen
        var newParagraph = document.createElement('p');
        var text = document.createTextNode(text);
        newParagraph.appendChild(text);
        document.getElementById('sentances').appendChild(newParagraph);
    }
}
