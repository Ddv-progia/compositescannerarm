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

ProcessingParameters ScanCollector::getProcessingParameters() const
{
	if (m_scanArm.get() != nullptr)	return m_scanArm->parameters;
	else return ProcessingParameters();
}

void ScanCollector::applyParameters(ProcessingParameters params)
{
	m_scanArm->parameters = params;
}

void ScanCollector::share() {
	emit ready(m_scanArm);
}
