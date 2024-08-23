/*
 * Core/ScanCollector.cc
 */


#include "Core/ScanAlgorithms.hh"
#include "Core/ScanCollector.hh"
#include "Core/ScanIO.hh"
#include "Core/ScanProcessingTask.hh"

#include <iostream>


ScanCollector::ScanCollector(){ 
}

void ScanCollector::share() {
	emit ready(m_scanArm);
}
