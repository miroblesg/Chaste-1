/*

Copyright (C) University of Oxford, 2005-2011

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Chaste is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Chaste is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details. The offer of Chaste under the terms of the
License is subject to the License being interpreted in accordance with
English Law and subject to any action against the University of Oxford
being under the jurisdiction of the English Courts.

You should have received a copy of the GNU Lesser General Public License
along with Chaste. If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef TESTDELTANOTCHCELLCYCLEMODEL_HPP_
#define TESTDELTANOTCHCELLCYCLEMODEL_HPP_

#include <cxxtest/TestSuite.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <fstream>

#include "DeltaNotchCellCycleModel.hpp"
#include "AbstractCellBasedTestSuite.hpp"
#include "OutputFileHandler.hpp"
#include "CheckReadyToDivideAndPhaseIsUpdated.hpp"
#include "WildTypeCellMutationState.hpp"
#include "CellwiseData.hpp"

class TestDeltaNotchCellCycleModel : public AbstractCellBasedTestSuite
{
public:

    ///\todo test correct behaviour of ODE system and state variables
    void TestCorrectBehaviour() throw(Exception)
    {
        std::vector<double> constant_data(1.0);
        CellwiseData<1>::Instance()->SetConstantDataForTesting(constant_data);
        CellwiseData<2>::Instance()->SetConstantDataForTesting(constant_data);
        CellwiseData<3>::Instance()->SetConstantDataForTesting(constant_data);

        TS_ASSERT_THROWS_NOTHING(DeltaNotchCellCycleModel cell_model3);

        DeltaNotchCellCycleModel* p_stem_model = new DeltaNotchCellCycleModel;
        p_stem_model->SetCellProliferativeType(STEM);
        p_stem_model->SetDimension(2);

        // Change G1 duration for this model
        p_stem_model->SetStemCellG1Duration(8.0);

        DeltaNotchCellCycleModel* p_transit_model = new DeltaNotchCellCycleModel;
        p_transit_model->SetCellProliferativeType(TRANSIT);
        p_transit_model->SetDimension(3);

        // Change G1 duration for this model
        p_stem_model->SetTransitCellG1Duration(8.0);

        DeltaNotchCellCycleModel* p_diff_model = new DeltaNotchCellCycleModel;
        p_diff_model->SetCellProliferativeType(DIFFERENTIATED);
        p_diff_model->SetDimension(1);

        boost::shared_ptr<AbstractCellMutationState> p_healthy_state(new WildTypeCellMutationState);

        CellPtr p_stem_cell(new Cell(p_healthy_state, p_stem_model));
        p_stem_cell->InitialiseCellCycleModel();

        CellPtr p_transit_cell(new Cell(p_healthy_state, p_transit_model));
        p_transit_cell->InitialiseCellCycleModel();

        CellPtr p_diff_cell(new Cell(p_healthy_state, p_diff_model));
        p_diff_cell->InitialiseCellCycleModel();

        SimulationTime* p_simulation_time = SimulationTime::Instance();
        unsigned num_steps = 100;
        double end_time = 2.0*(p_stem_model->GetStemCellG1Duration() + p_stem_model->GetSG2MDuration());
        p_simulation_time->SetEndTimeAndNumberOfTimeSteps(end_time, num_steps);

        for (unsigned i=0; i<num_steps; i++)
        {
            p_simulation_time->IncrementTimeOneStep();

            // The numbers for the G1 durations below are taken from the first three random numbers generated
            CheckReadyToDivideAndPhaseIsUpdated(p_stem_model, 4.36075);
            CheckReadyToDivideAndPhaseIsUpdated(p_transit_model, 1.78877);
            CheckReadyToDivideAndPhaseIsUpdated(p_diff_model, 132);  // any old number
        }

        // Coverage of Get / Set Methods
        TS_ASSERT_DELTA(p_diff_model->GetNotch(), 0.0, 1e-4);
        TS_ASSERT_DELTA(p_diff_model->GetDelta(), 1.0, 1e-4);
        TS_ASSERT_DELTA(p_diff_model->GetMeanNeighbouringDelta(), 0.0, 1e-4);

        // Tidy up
        CellwiseData<2>::Destroy();
    }

    ///\todo test archiving of ODE system and state variables
    void TestArchiveDeltaNotchCellCycleModel()
    {
        std::vector<double> constant_data(1.0);
        CellwiseData<2>::Instance()->SetConstantDataForTesting(constant_data);

        OutputFileHandler handler("archive", false);
        std::string archive_filename = handler.GetOutputDirectoryFullPath() + "delta_notch_cell_cycle.arch";

        double random_number_test = 0;

        // Create an output archive
        {
            SimulationTime* p_simulation_time = SimulationTime::Instance();
            p_simulation_time->SetEndTimeAndNumberOfTimeSteps(2.0, 4);

            DeltaNotchCellCycleModel* p_model = new DeltaNotchCellCycleModel;
            p_model->SetDimension(2);
            p_model->SetCellProliferativeType(TRANSIT);
            boost::shared_ptr<AbstractCellMutationState> p_healthy_state(new WildTypeCellMutationState);
            CellPtr p_cell(new Cell(p_healthy_state, p_model));
            p_cell->InitialiseCellCycleModel();
            p_cell->SetBirthTime(-1.1);
            p_simulation_time->IncrementTimeOneStep();
            p_simulation_time->IncrementTimeOneStep();

            p_cell->ReadyToDivide(); // updates phases

            std::ofstream ofs(archive_filename.c_str());
            boost::archive::text_oarchive output_arch(ofs);

            TS_ASSERT_DELTA(p_model->GetSDuration(), 5.0, 1e-12);

            CellPtr const p_const_cell = p_cell;
            output_arch << p_const_cell;

            TS_ASSERT_DELTA(p_model->GetBirthTime(), -1.1, 1e-12);
            TS_ASSERT_DELTA(p_model->GetAge(), 2.1, 1e-12);
            TS_ASSERT_EQUALS(p_model->GetCurrentCellCyclePhase(), G_ONE_PHASE);
            TS_ASSERT_EQUALS(p_model->GetCellProliferativeType(), TRANSIT);

            RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();
            random_number_test = p_gen->ranf();
            SimulationTime::Destroy();
        }

        {
            SimulationTime* p_simulation_time = SimulationTime::Instance();
            p_simulation_time->SetStartTime(0.0);
            p_simulation_time->SetEndTimeAndNumberOfTimeSteps(1.0, 1);

            RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();
            p_gen->Reseed(128);

            CellPtr p_cell;

            // Create an input archive
            std::ifstream ifs(archive_filename.c_str(), std::ios::binary);
            boost::archive::text_iarchive input_arch(ifs);

            // Restore from the archive
            input_arch >> p_cell;

            TS_ASSERT_DELTA(RandomNumberGenerator::Instance()->ranf(), random_number_test, 1e-7);

            AbstractCellCycleModel* p_model = p_cell->GetCellCycleModel();

            // Test that the cell-cycle model was restored correctly
            TS_ASSERT_DELTA(p_model->GetBirthTime(), -1.1, 1e-12);
            TS_ASSERT_DELTA(p_model->GetAge(), 2.1, 1e-12);
            TS_ASSERT_EQUALS(p_model->GetCurrentCellCyclePhase(), G_ONE_PHASE);
            TS_ASSERT_EQUALS(p_model->GetCellProliferativeType(), TRANSIT);
            TS_ASSERT_DELTA(p_model->GetSDuration(), 5.0, 1e-12);
        }

        // Tidy up
        CellwiseData<1>::Destroy();
    }

    void TestCellCycleModelOutputParameters()
    {
        std::string output_directory = "TestCellCycleModelOutputParameters";
        OutputFileHandler output_file_handler(output_directory, false);

        // Test with DeltaNotchCellCycleModel
        DeltaNotchCellCycleModel cell_cycle_model;
        TS_ASSERT_EQUALS(cell_cycle_model.GetIdentifier(), "DeltaNotchCellCycleModel");

        out_stream parameter_file = output_file_handler.OpenOutputFile("delta_notch_results.parameters");
        cell_cycle_model.OutputCellCycleModelParameters(parameter_file);
        parameter_file->close();

        std::string results_dir = output_file_handler.GetOutputDirectoryFullPath();
       // TS_ASSERT_EQUALS(system(("diff " + results_dir + "delta_notch_results.parameters notforrelease_cell_based/test/data/TestDeltaNotchCellCycleModel/delta_notch_results.parameters").c_str()), 0);
    }

    void TestCreateCopyCellCycleModel()
    {
        // Test with DeltaNotchCellCycleModel
        DeltaNotchCellCycleModel* p_model= new DeltaNotchCellCycleModel;

        // Set ODE system
        double mean_delta=1.0;
        std::vector<double> state_variables;
        state_variables.push_back(1.0);
        state_variables.push_back(1.0);
        state_variables.push_back(1.0);
        p_model->SetOdeSystem(new DeltaNotchOdeSystem(mean_delta, state_variables));

        // Set model parameters.
        p_model->SetBirthTime(2.0);
        p_model->SetDimension(2);
        p_model->SetGeneration(2);
        p_model->SetMaxTransitGenerations(10);
        p_model->SetCellProliferativeType(STEM);

        // Create a copy
        DeltaNotchCellCycleModel* p_model2 = static_cast<DeltaNotchCellCycleModel*> (p_model->CreateCellCycleModel());

        // Check correct initializations
		TS_ASSERT_EQUALS(p_model2->GetBirthTime(),2);
        TS_ASSERT_EQUALS(p_model2->GetDimension(), 2u);
        TS_ASSERT_EQUALS(p_model2->GetGeneration(), 2u);
        TS_ASSERT_EQUALS(p_model2->GetMaxTransitGenerations(), 10u);
        TS_ASSERT_EQUALS(p_model2->GetCellProliferativeType(), STEM);

        // Destruct models.
        delete p_model;
        delete p_model2;
    }
};

#endif /* TESTDELTANOTCHCELLCYCLEMODEL_HPP_ */