#ifndef UNIT_H
#define UNIT_H

enum Unit
{
  Metric = 1,
  Imperial = 2
};

Unit validateUnit(int);

#endif