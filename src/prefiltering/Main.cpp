
#include "Prefiltering.h"
#include "Util.h"
#include "Parameters.h"
#include "MMseqsMPI.h"
#include "Timer.h"

#include <iostream>
#include <string>

#ifdef OPENMP
#include <omp.h>
#endif

int prefilter(int argc, const char **argv, const Command& command) {
    MMseqsMPI::init(argc, argv);

    Parameters& par = Parameters::getInstance();
    par.parseParameters(argc, argv, command, 3, true, 0, MMseqsParameter::COMMAND_PREFILTER);

    Timer timer;
    Debug(Debug::INFO) << "Initialising data structures...\n";

    int queryDbType = DBReader<unsigned int>::parseDbType(par.db1.c_str());
    int targetDbType = DBReader<unsigned int>::parseDbType(par.db2.c_str());
    if (queryDbType == -1 || targetDbType == -1) {
        Debug(Debug::ERROR) << "Please recreate your database or add a .dbtype file to your sequence/profile database.\n";
        return EXIT_FAILURE;
    }
    if (Parameters::isEqualDbtype(queryDbType, Parameters::DBTYPE_HMM_PROFILE) && Parameters::isEqualDbtype(targetDbType, Parameters::DBTYPE_HMM_PROFILE)) {
        Debug(Debug::ERROR) << "Only the query OR the target database can be a profile database.\n";
        return EXIT_FAILURE;
    }
    if (Parameters::isEqualDbtype(queryDbType, Parameters::DBTYPE_HMM_PROFILE) == false && Parameters::isEqualDbtype(targetDbType, Parameters::DBTYPE_PROFILE_STATE_SEQ)) {
        Debug(Debug::ERROR) << "The query has to be a profile when using a target profile state database.\n";
        return EXIT_FAILURE;
    } else if (Parameters::isEqualDbtype(queryDbType, Parameters::DBTYPE_HMM_PROFILE) && Parameters::isEqualDbtype(targetDbType, Parameters::DBTYPE_PROFILE_STATE_SEQ)) {
        queryDbType = Parameters::DBTYPE_PROFILE_STATE_PROFILE;
    }

    Prefiltering pref(par.db2, par.db2Index, queryDbType, targetDbType, par);
    Debug(Debug::INFO) << "Time for init: " << timer.lap() << "\n";

#ifdef HAVE_MPI
    pref.runMpiSplits(par.db1, par.db1Index, par.db3, par.db3Index, par.localTmp);
#else
    pref.runAllSplits(par.db1, par.db1Index, par.db3, par.db3Index);
#endif

    return EXIT_SUCCESS;
}
