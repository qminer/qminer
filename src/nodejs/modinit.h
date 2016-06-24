/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef QM_MODINIT_H_
#define QM_MODINIT_H_

#ifndef BUILDING_NODE_EXTENSION
	#define BUILDING_NODE_EXTENSION
#endif

#include <node.h>
#include <node_object_wrap.h>
#include <node_buffer.h>
#include <uv.h>
#include "base.h"
#include <qminer.h>
#include <thread.h>

#include "streamstory.h"
#include "Snap.h"

#include "nodeutil.h"
#include "fs_nodejs.h"
#include "la_structures_nodejs.h"
#include "la_vector_nodejs.h"
#include "la_nodejs.h"
#include "ht_nodejs.h"
#include "analytics.h"
#include "stat_nodejs.h"
#include "snap_nodejs.h"
#include "qm_nodejs_streamaggr.h"
#include "qm_nodejs_store.h"
#include "qm_nodejs.h"
#include "streamstory_node.h"

// include some implementations at the end, so we don't get incomplete types
#include "nodeutil.hpp"

#endif
