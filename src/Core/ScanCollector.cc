/*
 * Core/ScanCollector.cc
 */


#include "Core/ScanCollector.hh"
#include "Core/ScanProcessingTask.hh"
#include "Core/ScanIO.hh"
#include <iostream>


ScanCollector::ScanCollector(){ 

}

void ScanCollector::share() {
	emit ready(m_scan);
}