/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <heap.h>

void* heapAlloc(uint32_t sz)
{
  void* ret = NULL;

  ret = malloc((size_t)sz);

  return ret;
}

void heapFree(void* ptr)
{
  if (ptr == NULL) {
    // NULL is a valid reply from heapAlloc, and thus it is not an error for
    // us to receive it here.  We just ignore it.
    return;
  }

  free(ptr);
}

int heapFreeAll(uint32_t tid)
{
  return 0;
}

