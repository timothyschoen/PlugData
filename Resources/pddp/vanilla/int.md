---
title: int
description: store and recall an integer
categories:
- object
pdcategory: General
last_update: '0.48'
see_also:
- float
- value
- send
arguments:
- description: initially stored value 
  default: 0
  type: float
inlets:
  1st:
  - type: bang
    description: output the stored value.
  - type: list
    description: considers the first element if it's a float,  stores and outputs
      it.
  - type: send <symbol>
    description: send the stored value to a [receive] or [value] object that has the
      same name as the symbol (no output