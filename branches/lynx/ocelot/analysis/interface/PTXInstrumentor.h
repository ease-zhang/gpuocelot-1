/*! \file PTXInstrumentor.h
	\date Saturday November 13, 2010
	\author Naila Farooqui <naila@cc.gatech.edu>
	\brief The header file for the Ocelot PTX Instrumentor
*/

#ifndef PTX_INSTRUMENTOR_H_INCLUDED
#define PTX_INSTRUMENTOR_H_INCLUDED

#include <string>
#include <vector>
#include <ocelot/ir/interface/Module.h>
#include <ocelot/analysis/interface/Pass.h>

namespace analysis
{
	/*! \brief Able to run various instrumentation passes over PTX modules */
	class PTXInstrumentor
	{
		public:
			
			/*! \brief The input file being instrumented */
			std::string input;
			
			/*! \brief The output file being generated */
			std::string output;
            
            /*! \brief The PTX module being instrumented */
            ir::Module *module;
            
            /*! \brief The name of kernel being instrumented */
			std::string kernelName;        

            /*! \brief The name of the instrumentation pass */
            std::string passName;

            /*! \brief The instrumentation pass */
            analysis::Pass *pass;
            
            /*! \brief The number of CTAs specified for this execution */
            unsigned int ctas;

            /*! \brief The number of threads specified for this execution */
            unsigned int threads;
			
			
		public:
			
            /*! \brief The initialize method performs any necessary CUDA runtime initialization prior to instrumentation */
            virtual void initialize() = 0;

            /*! \brief The createPass method instantiates the instrumentation pass */
            virtual analysis::Pass *createPass() = 0;

            /*! \brief The analyze method performs any necessary static analysis */
            virtual void analyze(ir::Module &module) = 0;

            /*! \brief The finalize method performs any necessary CUDA runtime actions after instrumentation */
            virtual void finalize() = 0;

			/*! \brief Performs the instrumentation */
			void instrument();		

            /*! \brief Performs the instrumentation */
            void instrument(ir::Module& module);	
	};

    typedef std::vector< PTXInstrumentor *> PTXInstrumentorVector;
}

#endif