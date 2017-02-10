#ifndef __TIMERTASK_H__
#define __TIMERTASK_H__
#include "include.h"
#include "auto_sensor.h"
#include "database.h"
#include "taskque.h"

#define RPTCNT  12

typedef struct {
	int32_t task_id;
	time_t task_start, task_end;
	int32_t task_flowup, task_flowdown;
	int32_t task_lightup, task_lightdown;
	int32_t task_cnt, task_tm, task_status;
} tmparam_t;

typedef struct tmtask_t{
	int32_t Is_start;
	int32_t second;
	struct sql_t *sql;
	struct sensor_t *sensor;
	struct Queue_t *Queue;
	uint32_t task_starttm, task_startid[RPTCNT];
	uint32_t task_endtm, task_endid[RPTCNT];

	int (*tmtask_start)(struct tmtask_t*);
	int (*tmtask_stop)(struct tmtask_t*);
	int (*tmtask_exec)(struct tmtask_t*);
	int (*tmtask_update)(struct tmtask_t*);

	void (*tmtask_release)(struct tmtask_t**,int);
} tmtask_t;

extern tmtask_t *tmtask_init(struct tmtask_t*,sensor_t*,Queue_t*);
#endif
