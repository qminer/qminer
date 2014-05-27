mkdir db
cd db
del /s /q *.*
cd ..
del /q lock
qm config -overwrite
qm create -def=timeSeries.def

