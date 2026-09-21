#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

int execute_command(Command *cmd);
int execute_pipeline(Pipeline *pipeline);
void setup_background_handler(void);

#endif
