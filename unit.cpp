#include "unit.h"

bool isValidUnit(int unit)
{
  return (unit == Metric || unit == Imperial);
}

Unit validateUnit(int unit)
{
  if (isValidUnit(unit))
  {
    return static_cast<Unit>(unit);
  }
  else
  {
    return Unit::Metric;
  }
}