#!/bin/bash

function e {
    expand -t 4 $1 > $1_temp
    rm $1
    mv $1_temp $1
}

e glib/mine/signalproc.cpp
e glib/mine/signalproc.h

e qminer/qminer.h
e qminer/qminer_aggr.cpp
e qminer/qminer_aggr.h
e qminer/qminer_core.cpp
e qminer/qminer_core.h
e qminer/qminer_ftr.cpp
e qminer/qminer_ftr.h
e qminer/qminer_srv.cpp
e qminer/qminer_srv.h
e qminer/qminer_storage.cpp
e qminer/qminer_storage.h

e nodejs/qm/qm_nodejs.cpp
e nodejs/qm/qm_nodejs_streamaggr.cpp
#e nodejs/qm/qm_nodejs.h
e nodejs/qm/qm_nodejs_streamaggr.h
e nodejs/qm/qm_nodejs_store.cpp
e nodejs/qm/qm_param.h
e nodejs/qm/qm_nodejs_store.h

e nodejs/nodeutil.cpp 
e nodejs/nodeutil.h

e nodejs/analytics/analytics.cpp
#e nodejs/analytics/analytics.h
