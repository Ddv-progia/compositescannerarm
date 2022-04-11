/*
 * Core/InitializeMediaFoundationTask.cc
 */

#include <ObjBase.h>

#include "Core/InitializeMediaFoundationTask.hh"

void InitializeMediaFoundationTask::operator()()
{
  CoInitializeEx(NULL, COINIT_MULTITHREADED);
}
