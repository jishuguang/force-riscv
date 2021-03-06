//
// Copyright (C) [2020] Futurewei Technologies, Inc.
//
// FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
// FIT FOR A PARTICULAR PURPOSE.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include <OperandConstraintRISCV.h>

#include <Instruction.h>
#include <InstructionStructure.h>
#include <Constraint.h>
#include <Random.h>
#include <Log.h>
#include <Operand.h>

#include <Generator.h>
#include <Register.h>

#include <algorithm>

using namespace std;

/*!
  \file OperandConstraintRISCV.cc
  \brief Code supporting RISCV specific operand constraints
*/

namespace Force {

  void BaseOffsetBranchOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    BranchOperandConstraint::Setup(rGen, rInstr, rOperandStruct);

    auto branch_opr_struct = dynamic_cast<const BranchOperandStructure*>(&rOperandStruct);
    if (branch_opr_struct == nullptr) {
      LOG(fail) << "{BaseOffsetBranchOperandConstraint::Setup} expecting operand " << rOperandStruct.mName << " to be \"BranchOperandStructure\" type." << endl;
      FAIL("unexpected-operand-structure-type");
    }

    Operand* base_opr = rInstr.FindOperandMutable(branch_opr_struct->Base(), true);
    mpBase = dynamic_cast<RegisterOperand*>(base_opr);
    if (mpBase == nullptr) {
      LOG(fail) << "{BaseOffsetBranchOperandConstraint::Setup} expecting operand " << base_opr->Name() << " to be \"RegisterOperand\" type." << endl;
      FAIL("unexpected-operand-type");
    }

    Operand* offset_opr = rInstr.FindOperandMutable(branch_opr_struct->Offset(), true);
    mpOffset = dynamic_cast<ImmediateOperand*>(offset_opr);
    if (mpOffset == nullptr) {
      LOG(fail) << "{BaseOffsetBranchOperandConstraint::Setup} expecting operand " << offset_opr->Name() << " to be \"ImmediateOperand\" type." << endl;
      FAIL("unexpected-operand-type");
    }
  }

  void VectorDataTraits::SetTraits(const std::string& choiceText)
  {
    mTypeName = choiceText;
    if (choiceText == "8B") {
      mLanes = 8;
      mElementSize = 1;
    }
    else if (choiceText == "16B" || choiceText == "B") {
      mLanes = 16;
      mElementSize = 1;
    }
    else if (choiceText == "4H") {
      mLanes = 4;
      mElementSize = 2;
    }
    else if (choiceText == "8H" || choiceText == "H") {
      mLanes = 8;
      mElementSize = 2;
    }
    else if (choiceText == "2S") {
      mLanes = 2;
      mElementSize = 4;
    }
    else if (choiceText == "4S" || choiceText == "S") {
      mLanes = 4;
      mElementSize = 4;
    }
    else if (choiceText == "1D") {
      mLanes = 1;
      mElementSize = 8;
    }
    else if (choiceText == "2D" || choiceText == "D") {
      mLanes = 2;
      mElementSize = 8;
    }
    else if (choiceText == "1Q") {
      mLanes = 1;
      mElementSize = 16;
    }
    else if (choiceText == "4B") {
      mLanes = 4;
      mElementSize = 1;
    }
    else if (choiceText == "2H") {
        mLanes = 2;
        mElementSize = 2;
    }
    else {
      LOG(error) << "Unhandled data type \"" << choiceText << "\"." << endl;
    }

    mVectorSize = mLanes * mElementSize;
  }

  void ConditionalBranchOperandRISCVConstraint::Setup(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct)
  {
    PcRelativeBranchOperandConstraint::Setup(gen, instr, operandStruct);
    auto taken_constr = instr.ConditionTakenConstraint();
    if (taken_constr)
      mTaken = bool(taken_constr->ChooseValue());
    else
      mTaken = Random::Instance()->Random32(0, 1);
    // << "{ConditionalBranchOperandRISCVConstraint::Setup} condition branch is set to " << ((mTaken) ? "taken" : "not taken") << endl;
   }

  void FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBEQ(uint64 rs1Val, uint64 rs2Val)
  {
    LOG(info) << "FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBEQ(...)" << endl;
    LOG(info) << "Rs1 val is: " << hex << rs1Val << " and the rs2 val is: " << rs2Val << endl;
    //Evaluate the BEQ logic here
    mTaken = (rs2Val == rs1Val);
  }

  void FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBNE(uint64 rs1Val, uint64 rs2Val)
  {
    LOG(info) << "FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBNE(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << " and the rs2 val is: " << rs2Val << endl;
    //Evaluate the BNE logic here
    mTaken = (rs2Val != rs1Val);
  }

  void FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBLT(int64 rs1Val, int64 rs2Val)
  {
    LOG(info) << "FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBLT(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << " and the rs2 val is: " << rs2Val << endl;
    //Evaluate the BLT logic here
    mTaken = (rs1Val < rs2Val);
  }

  void FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBLTU(uint64 rs1Val, uint64 rs2Val)
  {
    LOG(info) << "FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBLTU(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << " and the rs2 val is: " << rs2Val << endl;
    //Evaluate the BLTU logic here
    mTaken = (rs1Val < rs2Val);
  }

  void FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBGE(int64 rs1Val, int64 rs2Val)
  {
    LOG(info) << "FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBGE(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << " and the rs2 val is: " << rs2Val << endl;
    //Evaluate the BGE logic here
    mTaken = (rs1Val >= rs2Val);
  }

  void FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBGEU(uint64 rs1Val, uint64 rs2Val)
  {
    LOG(info) << "FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBGEU(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << " and the rs2 val is: " << rs2Val << endl;
    //Evaluate the BGEU logic here
    mTaken = (rs1Val >= rs2Val);
  }

  void CompressedConditionalBranchOperandConstraint::SetBranchTakenForCBEQZ(uint64 rs1Val)
  {
    LOG(info) << "CompressedConditionalBranchOperandConstraint::SetBranchTakenForCBEQZ(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << endl;
    //Evaluate the CBEQZ logic here
    mTaken = (rs1Val == 0);
  }

  void CompressedConditionalBranchOperandConstraint::SetBranchTakenForCBNEZ(uint64 rs1Val)
  {
    LOG(info) << "CompressedConditionalBranchOperandConstraint::SetBranchTakenForBNEZ(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << endl;
    //Evaluate the CBNEZ logic here
    mTaken = (rs1Val != 0);
  }

  void CompressedConditionalBranchOperandConstraint::SetConditionalBranchTaken(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct)
  {
    if (gen.SimulationEnabled()) 
    {
      // << "Simulation is enabled" << endl;
      string instr_name = instr.Name();
      
      auto branch_opr_struct = dynamic_cast<const BranchOperandStructure*>(&operandStruct);
      if (branch_opr_struct == nullptr) {
        LOG(fail) << "{CompressedConditionalBranchOperandConstraint::Setup} expecting operand " << operandStruct.mName << " to be \"BranchOperandStructure\" type." << endl;
        FAIL("unexpected-operand-structure-type");
      }
      EBranchConditionType branch_type = branch_opr_struct->mCondition;

      //Obtain handles to operands
      bool fail_if_not_found = true;
      const Operand*  src_1_opr = instr.FindOperand("rs1'", fail_if_not_found);
      const RegisterFile* reg_file = gen.GetRegisterFile();
      uint64 rs1_val = 0;

      //The rs1_val is alternatively used for rs1' if it is present.
      auto rs1_ropr = dynamic_cast<const RegisterOperand* >(src_1_opr);
      string rs1_reg_name = rs1_ropr->ChoiceText();
      rs1_val = reg_file->RegisterLookup(rs1_reg_name)->Value();
 
      switch(branch_type)
      {
        case EBranchConditionType::CBEQZ: SetBranchTakenForCBEQZ(rs1_val); break;
        case EBranchConditionType::CBNEZ: SetBranchTakenForCBNEZ(rs1_val); break;
        default:
          LOG(fail) << "{CompressedConditionalBranchOperandConstraint::SetConditionalBranchTaken} unknown branch instruction:" << instr_name << endl;
          FAIL("unknown-branch_instruction");
      } 
    }
    auto taken_constr = instr.ConditionTakenConstraint();
    if (taken_constr && bool(taken_constr->ChooseValue()) != mTaken) {
      LOG(fail) << "{CompressedConditionalBranchOperandConstraint::SetConditionalBranchTaken} not resolved condition taken constraint:" << taken_constr->ChooseValue() << endl;
      FAIL("unresolved-condition-taken-constraint");
    }
    LOG(notice) << "{CompressedConditionalBranchOperandConstraint::SetConditionalBranchTaken} condition branch is set to " << ((mTaken) ? "taken" : "not taken") << endl;
  }

  void FullsizeConditionalBranchOperandConstraint::SetConditionalBranchTaken(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct)
  {
    if (gen.SimulationEnabled()) 
    {
      // << "Simulation is enabled" << endl;
      string instr_name = instr.Name();
      
      auto branch_opr_struct = dynamic_cast<const BranchOperandStructure*>(&operandStruct);
      if (branch_opr_struct == nullptr) {
        LOG(fail) << "{FullsizeConditionalBranchOperandConstraint::Setup} expecting operand " << operandStruct.mName << " to be \"BranchOperandStructure\" type." << endl;
        FAIL("unexpected-operand-structure-type");
      }
      EBranchConditionType branch_type = branch_opr_struct->mCondition;

      //Obtain handles to operands
      bool fail_if_not_found = true;
      const Operand* src_2_opr = instr.FindOperand("rs2", fail_if_not_found);
      const Operand* src_1_opr = instr.FindOperand("rs1", fail_if_not_found);
      const RegisterFile* reg_file = gen.GetRegisterFile();

      uint64 rs2_val = 0;
      uint64 rs1_val = 0;
    
      auto rs2_ropr = dynamic_cast<const RegisterOperand* >(src_2_opr);
      string rs2_reg_name = rs2_ropr->ChoiceText();
      rs2_val = reg_file->RegisterLookup(rs2_reg_name)->Value();

      auto rs1_ropr = dynamic_cast<const RegisterOperand* >(src_1_opr);
      string rs1_reg_name = rs1_ropr->ChoiceText();
      rs1_val = reg_file->RegisterLookup(rs1_reg_name)->Value();
 
      switch(branch_type)
      {
        case EBranchConditionType::BEQ:  SetBranchTakenForBEQ(rs1_val, rs2_val); break; 
        case EBranchConditionType::BNE:  SetBranchTakenForBNE(rs1_val, rs2_val); break;
        case EBranchConditionType::BLTU: SetBranchTakenForBLTU(rs1_val, rs2_val); break; 
        case EBranchConditionType::BLT:  SetBranchTakenForBLT(rs1_val, rs2_val); break; 
        case EBranchConditionType::BGEU: SetBranchTakenForBGEU(rs1_val, rs2_val); break;         
        case EBranchConditionType::BGE:  SetBranchTakenForBGE(rs1_val, rs2_val); break;
        default:
          LOG(fail) << "{FullsizeConditionalBranchOperandConstraint::SetConditionalBranchTaken} unknown branch instruction:" << instr_name << endl;
          FAIL("unknown-branch_instruction");
      } 
    }
    auto taken_constr = instr.ConditionTakenConstraint();
    if (taken_constr && bool(taken_constr->ChooseValue()) != mTaken) {
      LOG(fail) << "{FullsizeConditionalBranchOperandConstraint::SetConditionalBranchTaken} not resolved condition taken constraint:" << taken_constr->ChooseValue() << endl;
      FAIL("unresolved-condition-taken-constraint");
    }
    LOG(notice) << "{FullsizeConditionalBranchOperandConstraint::SetConditionalBranchTaken} condition branch is set to " << ((mTaken) ? "taken" : "not taken") << endl;
  }

  void RISCVectorRegisterOperandConstraint::Setup(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct)
  {
    RegisterOperandConstraint::Setup(gen, instr, operandStruct);
  }

  void VectorLoadStoreOperandConstraint::Setup(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct)
  {
    BaseOffsetLoadStoreOperandConstraint::Setup(gen, instr, operandStruct);
    auto vls_struct = dynamic_cast<const LoadStoreOperandStructure* >(&operandStruct);
    if (nullptr == vls_struct) {
      LOG(fail) << "{VectorLoadStoreOperandConstraint::Setup} expecting operand " << operandStruct.mName << " to be \"LoadStoreOperandStructure\" type." << endl;
      FAIL("expecting-vector-load-store-operand-structure");
    }
    auto base_ptr = instr.FindOperandMutable(vls_struct->Base(), true);
    mpBase = dynamic_cast<RegisterOperand* >(base_ptr);
    if (nullptr == mpBase) {
      LOG(fail) << "{VectorLoadStoreOperandConstraint::Setup} expecting operand " << base_ptr->Name() << " to be \"RegisterOperand\" type." << endl;
      FAIL("expecting-register_operand");
    }
    //TODO: properly address the MultiRegisterOperand
    auto multi_ptr = instr.FindOperand("vd");
    mpMultiRegisterOperand = dynamic_cast<const MultiRegisterOperand* >(multi_ptr);
    if (nullptr == mpMultiRegisterOperand) {
      LOG(fail) << "{VectorLoadStoreOperandConstraint::Setup} expecting operand " << multi_ptr->Name() << " to be \"MultiRegisterOperand\" type." << endl;
      FAIL("expecting-register_operand");
    }

    string vector_name[] = {"rs1", "vs3", "vd", "\0"};
    for (unsigned i = 0; vector_name[i] != "\0"; i ++) {
        auto vector_ptr = instr.FindOperand(vector_name[i]);
        if (vector_ptr) {
          mpVectorDataType = dynamic_cast<const ChoicesOperand* >(vector_ptr);
          if (nullptr == mpVectorDataType) {
            LOG(fail) << "{VectorLoadStoreOperandConstraint::Setup} expecting operand " << vector_ptr->Name() << " to be \"ChoicesOperand\" type." << endl;
            FAIL("expecting-choices_operand");
          }
          return;
        }
    }

    LOG(fail) << "{VectorLoadStoreOperandConstraint::Setup} expecting operand to be \"ChoicesOperand\" type." << endl;
    FAIL("expecting-choices_operand");
  }

}
