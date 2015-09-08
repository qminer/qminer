var fs = require("fs");

var wstream = fs.createWriteStream('people_huge.json');
for (var i=0; i<100*1000; i++){
	var obj={ 
		Name: "Person "+i,
	   Gender: Math.random() > 0.54 ? "Male":"Female" 
	}
	wstream.write(JSON.stringify(obj) +'\n');	
}
wstream.end();

var wstream = fs.createWriteStream('people_big.json');
for (var i=0; i<1000; i++){
	var obj={ 
		Name: "Person "+i,
	   Gender: Math.random() > 0.54 ? "Male":"Female" 
	}
	wstream.write(JSON.stringify(obj) +'\n');	
}
wstream.end();

var wstream = fs.createWriteStream('people_small.json');
for (var i=0; i<2; i++){
	var obj={ 
		Name: "Person "+i,
	   Gender: Math.random() > 0.54 ? "Male":"Female" 
	}
	wstream.write(JSON.stringify(obj) +'\n');	
}
wstream.end();