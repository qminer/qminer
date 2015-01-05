var sys=require('sys');
var count = 0;

sys.debug("Starting ...");


function timer_tick() {
  count = count+1;
  sys.debug("Tick count: " + count);
  if (count === 10) {
    count += 1000;
    sys.debug("Set break here");
 }
 setTimeout(timer_tick, 1000);
}

timer_tick();