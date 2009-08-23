/*!
	\file PTXToLLVMTranslator.cpp
	\date Wednesday July 29, 2009
	\author Gregory Diamos <gregory.diamos@gatech.edu>
	\brief The source file for the PTXToLLVMTranslator class
	\comment : Written with subdued haste
*/

#ifndef PTX_TO_LLVM_TRANSLATOR_CPP_INCLUDED
#define PTX_TO_LLVM_TRANSLATOR_CPP_INCLUDED

#include <ocelot/translator/interface/PTXToLLVMTranslator.h>
#include <ocelot/ir/interface/LLVMInstruction.h>
#include <ocelot/ir/interface/LLVMKernel.h>
#include <ocelot/ir/interface/PTXInstruction.h>

#include <queue>

#include <hydrazine/implementation/debug.h>

#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 1

namespace translator
{
	ir::LLVMInstruction::Operand 
		PTXToLLVMTranslator::_translate( const ir::PTXOperand& o )
	{
		ir::LLVMInstruction::Operand op( o.identifier, 
			o.addressMode == ir::PTXOperand::Immediate );
		
		switch( o.addressMode )
		{
			case ir::PTXOperand::Register: /* fall through */
			case ir::PTXOperand::Indirect:
			{
				std::stringstream stream;
				stream << "$r" << o.reg;
				op.name = stream.str();
				break;
			}
			case ir::PTXOperand::Immediate:
			{
				switch( o.type )
				{
					case ir::PTXOperand::s8: /* fall through */
					case ir::PTXOperand::s16: /* fall through */
					case ir::PTXOperand::s32: /* fall through */
					case ir::PTXOperand::s64: /* fall through */
					case ir::PTXOperand::u8: /* fall through */
					case ir::PTXOperand::u16: /* fall through */
					case ir::PTXOperand::u32: /* fall through */
					case ir::PTXOperand::u64: /* fall through */
					case ir::PTXOperand::b8: /* fall through */
					case ir::PTXOperand::b16: /* fall through */
					case ir::PTXOperand::b32: /* fall through */
					case ir::PTXOperand::b64:
					{
						op.i64 = o.imm_uint;
						break;
					}
					case ir::PTXOperand::f32:
					{
						op.f32 = o.imm_float;
						break;
					}
					case ir::PTXOperand::f64:
					{
						op.f64 = o.imm_float;
						break;
					}
					default:
					{
						assertM( false, "PTXOperand datatype " 
							+ ir::PTXOperand::toString( o.type ) 
							+ " not supported for immediate operand." );
					}				
				}
				break;
			}
			case ir::PTXOperand::Address:
			{
				break;
			}
			case ir::PTXOperand::Label:
			{
				assertM( false, "PTXOperand datatype " 
					+ ir::PTXOperand::toString( o.type ) 
					+ " not supported." );
				break;
			}
			case ir::PTXOperand::Special:
			{
				op.name = ir::PTXOperand::toString( o.special );
				break;
			}
			case ir::PTXOperand::Invalid:
			{
				assertM( false, "Cannot translate invalid PTX operand." );
			}
		}

		switch( o.type )
		{
			case ir::PTXOperand::pred:
			{
				op.type.type = ir::LLVMInstruction::I1;
				break;
			}			
			case ir::PTXOperand::b8: /* fall through */
			case ir::PTXOperand::u8: /* fall through */
			case ir::PTXOperand::s8:
			{
				op.type.type = ir::LLVMInstruction::I8;
				break;
			}
			case ir::PTXOperand::b16: /* fall through */
			case ir::PTXOperand::s16: /* fall through */
			case ir::PTXOperand::u16:
			{
				op.type.type = ir::LLVMInstruction::I16;
				break;
			}
			case ir::PTXOperand::b32: /* fall through */
			case ir::PTXOperand::u32: /* fall through */
			case ir::PTXOperand::s32:
			{
				op.type.type = ir::LLVMInstruction::I32;
				break;
			}
			case ir::PTXOperand::b64: /* fall through */
			case ir::PTXOperand::s64: /* fall through */
			case ir::PTXOperand::u64:
			{
				op.type.type = ir::LLVMInstruction::I64;
				break;
			}
			case ir::PTXOperand::f32:
			{
				op.type.type = ir::LLVMInstruction::F32;
				break;
			}
			case ir::PTXOperand::f64:
			{
				op.type.type = ir::LLVMInstruction::F64;
				break;
			}
			default:
			{
				assertM( false, "PTXOperand datatype " 
					+ ir::PTXOperand::toString( o.type ) 
					+ " not supported." );
			}				
		}

		if( o.vec == ir::PTXOperand::v1 )
		{
			op.type.category = ir::LLVMInstruction::Type::Element;
		}
		else
		{
			op.type.category = ir::LLVMInstruction::Type::Vector;
		}
		
		op.type.vector = o.vec;
		
		return op;
	}

	void PTXToLLVMTranslator::_swapAllExceptName( 
		ir::LLVMInstruction::Operand& o, const ir::PTXOperand& i )
	{
		std::string temp = o.name;
		o = _translate( i );
		o.name = temp;
	}

	void PTXToLLVMTranslator::_doubleWidth( ir::LLVMInstruction::DataType& t )
	{
		switch( t )
		{
			case ir::LLVMInstruction::I1:
			{
				assertM( false, "Cannot double i1" );
				break;
			}
			case ir::LLVMInstruction::I8:
			{
				t = ir::LLVMInstruction::I16;
				break;
			}
			case ir::LLVMInstruction::I16:
			{
				t = ir::LLVMInstruction::I32;
				break;
			}
			case ir::LLVMInstruction::I32:
			{
				t = ir::LLVMInstruction::I64;
				break;
			}
			case ir::LLVMInstruction::I64:
			{
				assertM( false, "Cannot double i64" );
				break;
			}
			case ir::LLVMInstruction::F32:
			{
				assertM( false, "Cannot double f32" );
				break;
			}
			case ir::LLVMInstruction::F64:
			{
				assertM( false, "Cannot double f64" );
				break;
			}
			case ir::LLVMInstruction::InvalidDataType:
			{
				assertM( false, "Cannot double invalid data type" );
				break;
			}
		}
	}

	ir::LLVMInstruction::Comparison PTXToLLVMTranslator::_translate( 
		ir::PTXInstruction::CmpOp op, bool isInt )
	{
		if( isInt )
		{
			switch( op )
			{
				case ir::PTXInstruction::Eq: 
					return ir::LLVMInstruction::Eq; break;
				case ir::PTXInstruction::Ne: 
					return ir::LLVMInstruction::Ne; break;
				case ir::PTXInstruction::Lo: /* fall through */
				case ir::PTXInstruction::Lt: 
					return ir::LLVMInstruction::Slt; break;
				case ir::PTXInstruction::Ls: /* fall through */
				case ir::PTXInstruction::Le: 
					return ir::LLVMInstruction::Sle; break;
				case ir::PTXInstruction::Hi: /* fall through */
				case ir::PTXInstruction::Gt: 
					return ir::LLVMInstruction::Sgt; break;
				case ir::PTXInstruction::Hs: /* fall through */
				case ir::PTXInstruction::Ge: 
					return ir::LLVMInstruction::Sge; break;
				default: assertM( false, "Invalid comparison " 
					<< ir::PTXInstruction::toString( op ) 
					<< " for integer operand." );
			}
		}
		else
		{
			switch( op )
			{
				case ir::PTXInstruction::Eq: 
					return ir::LLVMInstruction::Oeq; break;
				case ir::PTXInstruction::Ne: 
					return ir::LLVMInstruction::One; break;
				case ir::PTXInstruction::Lt: 
					return ir::LLVMInstruction::Olt; break;
				case ir::PTXInstruction::Le: 
					return ir::LLVMInstruction::Ole; break;
				case ir::PTXInstruction::Gt: 
					return ir::LLVMInstruction::Ogt; break;
				case ir::PTXInstruction::Ge: 
					return ir::LLVMInstruction::Oge; break;
				case ir::PTXInstruction::Lo: 
					return ir::LLVMInstruction::Olt; break;
				case ir::PTXInstruction::Ls: 
					return ir::LLVMInstruction::Ole; break;
				case ir::PTXInstruction::Hi: 
					return ir::LLVMInstruction::Ogt; break;
				case ir::PTXInstruction::Hs: 
					return ir::LLVMInstruction::Oge; break;
				case ir::PTXInstruction::Equ: 
					return ir::LLVMInstruction::Ueq; break;
				case ir::PTXInstruction::Neu: 
					return ir::LLVMInstruction::Une; break;
				case ir::PTXInstruction::Ltu: 
					return ir::LLVMInstruction::Ult; break;
				case ir::PTXInstruction::Leu: 
					return ir::LLVMInstruction::Ule; break;
				case ir::PTXInstruction::Gtu: 
					return ir::LLVMInstruction::Ugt; break;
				case ir::PTXInstruction::Geu: 
					return ir::LLVMInstruction::Uge; break;
				case ir::PTXInstruction::Num: 
					return ir::LLVMInstruction::Ord; break;
				case ir::PTXInstruction::Nan: 
					return ir::LLVMInstruction::Uno; break;
				default: assertM( false, "Invalid comparison " 
					<< ir::PTXInstruction::toString( op ) 
					<< " for floating point operand." );
			}
		}
		
		return ir::LLVMInstruction::True;
	}

	void PTXToLLVMTranslator::_yield()
	{
		ir::LLVMCall call;
		
		call.name = "translator::yield";
		call.functionAttributes = ir::LLVMInstruction::NoReturn;
		
		_add( call );
	}

	void PTXToLLVMTranslator::_convertPtxToSsa()
	{
		report( " Doing basic PTX register allocation");
		ir::Kernel::assignRegisters( _llvmKernel->instructions );
		report( " Building the dataflow graph");
		_graph = new analysis::DataflowGraph( *_llvmKernel->ptxCFG, 
			_llvmKernel->instructions );
		report( " Converting PTX to SSA form");
		_graph->toSsa();
	}

	void PTXToLLVMTranslator::_translateInstructions()
	{
		for( analysis::DataflowGraph::iterator block = _graph->begin(); 
			block != _graph->end(); ++block )
		{
			_newBlock( block->label() );
			report( "  Translating Phi Instructions" );
			for( analysis::DataflowGraph::PhiInstructionVector::const_iterator 
				phi = block->phis().begin(); 
				phi != block->phis().end(); ++phi )
			{
				ir::LLVMPhi p;
				analysis::DataflowGraph::RegisterIdVector::const_iterator 
					s = phi->s.begin();
				for( ; s != phi->s.end(); ++s )
				{			
					ir::LLVMPhi::Node node;
					node.label = block->producer( *s );
					node.reg = *s;

					std::stringstream stream;
					stream << "r" << *s;
					
					node.operand.name = stream.str();
					
					p.nodes.push_back( node );
				}

				assert( !p.nodes.empty() );
				
				std::stringstream stream;
				stream << "r" << phi->d;
				p.d.name = stream.str();
				
				_phiProducers.insert( std::make_pair( phi->d, 
					_llvmKernel->_statements.size() ) );
				report( "   Adding phi destination $" << stream.str() );
				_phiIndices.push_back( _llvmKernel->_statements.size() );
				_add( p );
			}
			report( "  Translating Instructions" );
			for( analysis::DataflowGraph::InstructionVector::const_iterator 
				instruction = block->instructions().begin();
				instruction != block->instructions().end(); ++instruction )
			{
				_instructionId = instruction->id;
				_translate( *instruction, *block );
			}
		}
	}

	void PTXToLLVMTranslator::_newBlock( const std::string& name )
	{
		report( " Translating basic block: " << name );
		_llvmKernel->_statements.push_back( ir::LLVMStatement( name ) );
	}

	void PTXToLLVMTranslator::_translate( 
		const analysis::DataflowGraph::Instruction& i, 
		const analysis::DataflowGraph::Block& block )
	{
		assert( i.id < _llvmKernel->instructions.size() );
		_translate( _llvmKernel->instructions[ i.id ], block );
	}

	void PTXToLLVMTranslator::_translate( const ir::PTXInstruction& i, 
		const analysis::DataflowGraph::Block& block )
	{
		report( "   Translating: " << i.toString() );
		assertM( i.valid() == "", "Instruction " << i.toString() 
			<< " is not valid: " << i.valid() );
		switch( i.opcode )
		{
			case ir::PTXInstruction::Abs: _translateAbs( i ); break;
			case ir::PTXInstruction::Add: _translateAdd( i ); break;
			case ir::PTXInstruction::AddC: _translateAddC( i ); break;
			case ir::PTXInstruction::And: _translateAnd( i ); break;
			case ir::PTXInstruction::Atom: _translateAtom( i ); break;
			case ir::PTXInstruction::Bar: _translateBar( i ); break;
			case ir::PTXInstruction::Bra: _translateBra( i, block ); break;
			case ir::PTXInstruction::Brkpt: _translateBrkpt( i ); break;
			case ir::PTXInstruction::Call: _translateCall( i ); break;
			case ir::PTXInstruction::CNot: _translateCNot( i ); break;
			case ir::PTXInstruction::Cos: _translateCos( i ); break;
			case ir::PTXInstruction::Cvt: _translateCvt( i ); break;
			case ir::PTXInstruction::Div: _translateDiv( i ); break;
			case ir::PTXInstruction::Ex2: _translateEx2( i ); break;
			case ir::PTXInstruction::Exit: _translateExit( i ); break;
			case ir::PTXInstruction::Ld: _translateLd( i ); break;
			case ir::PTXInstruction::Lg2: _translateLg2( i ); break;
			case ir::PTXInstruction::Mad24: _translateMad24( i ); break;
			case ir::PTXInstruction::Mad: _translateMad( i ); break;
			case ir::PTXInstruction::Max: _translateMax( i ); break;
			case ir::PTXInstruction::Membar: _translateMembar( i ); break;
			case ir::PTXInstruction::Min: _translateMin( i ); break;
			case ir::PTXInstruction::Mov: _translateMov( i ); break;
			case ir::PTXInstruction::Mul24: _translateMul24( i ); break;
			case ir::PTXInstruction::Mul: _translateMul( i ); break;
			case ir::PTXInstruction::Neg: _translateNeg( i ); break;
			case ir::PTXInstruction::Not: _translateNot( i ); break;
			case ir::PTXInstruction::Or: _translateOr( i ); break;
			case ir::PTXInstruction::Pmevent: _translatePmevent( i ); break;
			case ir::PTXInstruction::Rcp: _translateRcp( i ); break;
			case ir::PTXInstruction::Red: _translateRed( i ); break;
			case ir::PTXInstruction::Rem: _translateRem( i ); break;
			case ir::PTXInstruction::Ret: _translateRet( i ); break;
			case ir::PTXInstruction::Rsqrt: _translateRsqrt( i ); break;
			case ir::PTXInstruction::Sad: _translateSad( i ); break;
			case ir::PTXInstruction::SelP: _translateSelP( i ); break;
			case ir::PTXInstruction::Set: _translateSet( i ); break;
			case ir::PTXInstruction::SetP: _translateSetP( i ); break;
			case ir::PTXInstruction::Shl: _translateShl( i ); break;
			case ir::PTXInstruction::Shr: _translateShr( i ); break;
			case ir::PTXInstruction::Sin: _translateSin( i ); break;
			case ir::PTXInstruction::SlCt: _translateSlCt( i ); break;
			case ir::PTXInstruction::Sqrt: _translateSqrt( i ); break;
			case ir::PTXInstruction::St: _translateSt( i ); break;
			case ir::PTXInstruction::Sub: _translateSub( i ); break;
			case ir::PTXInstruction::SubC: _translateSubC( i ); break;
			case ir::PTXInstruction::Tex: _translateTex( i ); break;
			case ir::PTXInstruction::Trap: _translateTrap( i ); break;
			case ir::PTXInstruction::Vote: _translateVote( i ); break;
			case ir::PTXInstruction::Xor: _translateXor( i ); break;
			default:
			{
				assertM( false, "Opcode " 
					<< ir::PTXInstruction::toString( i.opcode ) 
					<< " not supported." );
				break;
			}
		}
	}

	void PTXToLLVMTranslator::_translateAbs( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateAdd( const ir::PTXInstruction& i )
	{
		if( ir::PTXOperand::isFloat( i.type ) )
		{
			ir::LLVMFadd add;
		
			add.d = _destination( i );
			add.a = _translate( i.a );
			add.b = _translate( i.b );
		
			_add( add );	
			
			if( i.modifier & ir::PTXInstruction::sat )
			{
				ir::LLVMFcmp compare;
				
				compare.d.name = _tempRegister();
				compare.d.type.type = ir::LLVMInstruction::I1;
				compare.d.type.category = ir::LLVMInstruction::Type::Element;
				compare.comparison = ir::LLVMInstruction::Ule;
				compare.a = add.d;
				compare.b.type.type = compare.a.type.type;
				compare.b.type.category = ir::LLVMInstruction::Type::Element;
				compare.b.constant = true;
				if( compare.b.type.type == ir::LLVMInstruction::F32 )
				{
					compare.b.f32 = 0;
				}
				else
				{
					compare.b.f64 = 0;
				}
				
				ir::LLVMSelect select;
				
				select.d.name = _tempRegister();
				select.d.type.type = add.d.type.type;
				select.d.type.category = add.d.type.category;
				select.condition = compare.d;
				select.a = compare.b;
				select.b = add.d;
				
				_add( compare );
				_add( select );
				_predicateEpilogue( i, select.d );
			}
			else
			{
				_predicateEpilogue( i, add.d );
			}
		}
		else
		{
			assertM( !(i.modifier & ir::PTXInstruction::sat), 
				"Saturation for ptx int add not supported." );

			ir::LLVMAdd add;
			
			add.d = _destination( i );
			add.a = _translate( i.a );
			add.b = _translate( i.b );
		
			_add( add );
			_predicateEpilogue( i, add.d );
		}
		
	}

	void PTXToLLVMTranslator::_translateAddC( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateAnd( const ir::PTXInstruction& i )
	{						
		ir::LLVMAnd And;
		
		And.d = _destination( i );
		And.a = _translate( i.a );
		And.b = _translate( i.b );

		_add( And );
		_predicateEpilogue( i, And.d );
	}

	void PTXToLLVMTranslator::_translateAtom( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateBar( const ir::PTXInstruction& i )
	{
		_yield();
	}

	void PTXToLLVMTranslator::_translateBra( const ir::PTXInstruction& i, 
		const analysis::DataflowGraph::Block& block )
	{
		ir::LLVMBr branch;
		branch.condition = _translate( i.pg );
		branch.iftrue = i.d.identifier;
		branch.iffalse = block.fallthrough()->label();
		_add( branch );
	}

	void PTXToLLVMTranslator::_translateBrkpt( const ir::PTXInstruction& i )
	{
		ir::LLVMCall call;
		
		call.name = "translator::breakpoint";
		
		_add( call );
	}

	void PTXToLLVMTranslator::_translateCall( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateCNot( const ir::PTXInstruction& i )
	{
		ir::LLVMSelect select;
		select.d = _destination( i );
		select.condition = _translate( i.a );
		select.a = select.condition;
		select.a.constant = true;
		select.a.i64 = 0;
		select.b = select.a;		
		select.b.i64 = -1;
	
		_add( select );
		_predicateEpilogue( i, select.d );
	}

	void PTXToLLVMTranslator::_translateCos( const ir::PTXInstruction& i )
	{
		ir::LLVMCall call;
		
		if( i.modifier & ir::PTXInstruction::ftz )
		{
			call.name = "translator::cosFtz";
		}
		else
		{
			call.name = "translator::cos";
		}
		
		call.d = _destination( i );
		call.parameters.resize( 1 );
		call.parameters[0] = _translate( i.a );
		
		_add( call );
		_predicateEpilogue( i, call.d );
	}

	void PTXToLLVMTranslator::_translateCvt( const ir::PTXInstruction& i )
	{
		switch( i.a.type )
		{
			case ir::PTXOperand::s8:
			{
				switch( i.type )
				{
					case ir::PTXOperand::pred:
					case ir::PTXOperand::s16:
					case ir::PTXOperand::b16:
					case ir::PTXOperand::u16:
					case ir::PTXOperand::s32:
					case ir::PTXOperand::b32:
					case ir::PTXOperand::u32:
					case ir::PTXOperand::b64:
					case ir::PTXOperand::s64:
					case ir::PTXOperand::u64:
					{
						ir::LLVMSext sext;
						sext.d = _destination( i );
						sext.a = _translate( i.a );
	
						_add( sext );
						_predicateEpilogue( i, sext.d );
						break;
					}
					case ir::PTXOperand::s8:
					case ir::PTXOperand::u8:
					case ir::PTXOperand::b8:
					{
						_translateMov( i );
						break;
					}
					case ir::PTXOperand::f16:
					case ir::PTXOperand::f64:
					case ir::PTXOperand::f32:
					{
						ir::LLVMSitofp sitofp;
						sitofp.d = _destination( i );
						sitofp.a = _translate( i.a );
						
						_add( sitofp );
						_predicateEpilogue( i, sitofp.d );
						break;
					}
					case ir::PTXOperand::TypeSpecifier_invalid:
					{
						assertM( false, "Convert " << i.toString() 
							<< " not implemented." );
						break;
					}
				}
				break;
			}
			case ir::PTXOperand::s16:
			{
				switch( i.type )
				{
					case ir::PTXOperand::s8:
					{
						ir::LLVMBitcast bitcast;
						bitcast.d = _destination( i );
						bitcast.a = _translate( i.a );
						
						_add( bitcast );
						_predicateEpilogue( i, bitcast.d );
						break;
					}
					case ir::PTXOperand::b32:
					case ir::PTXOperand::u32:
					case ir::PTXOperand::s32:
					case ir::PTXOperand::b64:
					case ir::PTXOperand::u64:
					case ir::PTXOperand::s64:
					{
						ir::LLVMSext sext;
						sext.d = _destination( i );
						sext.a = _translate( i.a );
						
						_add( sext );
						_predicateEpilogue( i, sext.d );
						break;
					}
					case ir::PTXOperand::pred:
					case ir::PTXOperand::b8:
					case ir::PTXOperand::u8:
					{
						ir::LLVMBitcast bitcast;
						bitcast.d = _destination( i );
						bitcast.a = _translate( i.a );
	
						_add( bitcast );
						_predicateEpilogue( i, bitcast.d );
						break;
					}
					case ir::PTXOperand::s16:
					case ir::PTXOperand::b16:
					case ir::PTXOperand::u16:
					{
						_translateMov( i );
						break;
					}
					case ir::PTXOperand::f16:
					case ir::PTXOperand::f64:
					case ir::PTXOperand::f32:
					{
						ir::LLVMSitofp sitofp;
						sitofp.d = _destination( i );
						sitofp.a = _translate( i.a );
						
						_add( sitofp );
						_predicateEpilogue( i, sitofp.d );
						break;
					}
					case ir::PTXOperand::TypeSpecifier_invalid:
					{
						assertM( false, "Convert " << i.toString() 
							<< " not implemented." );
						break;
					}
				}
				break;
			}
			case ir::PTXOperand::s32:
			{
				switch( i.type )
				{
					case ir::PTXOperand::pred:
					case ir::PTXOperand::b8:
					case ir::PTXOperand::u8:
					case ir::PTXOperand::s8:
					case ir::PTXOperand::b16:
					case ir::PTXOperand::u16:
					case ir::PTXOperand::s16:
					{
						ir::LLVMBitcast bitcast;
						bitcast.d = _destination( i );
						bitcast.a = _translate( i.a );
	
						_add( bitcast );
						_predicateEpilogue( i, bitcast.d );
						break;
					}
					case ir::PTXOperand::s32:
					case ir::PTXOperand::b32:
					case ir::PTXOperand::u32:
					{
						_translateMov( i );
						break;
					}
					case ir::PTXOperand::s64:
					case ir::PTXOperand::b64:
					case ir::PTXOperand::u64:
					{
						ir::LLVMSext sext;
						sext.d = _destination( i );
						sext.a = _translate( i.a );
						
						_add( sext );
						_predicateEpilogue( i, sext.d );
						break;
					}
					case ir::PTXOperand::f16:
					case ir::PTXOperand::f64:
					case ir::PTXOperand::f32:
					{
						ir::LLVMSitofp sitofp;
						sitofp.d = _destination( i );
						sitofp.a = _translate( i.a );
						
						_add( sitofp );
						_predicateEpilogue( i, sitofp.d );
						break;
					}
					case ir::PTXOperand::TypeSpecifier_invalid:
					{
						assertM( false, "Convert " << i.toString() 
							<< " not implemented." );
						break;
					}
				}
				break;
			}
			case ir::PTXOperand::s64:
			{
				switch( i.type )
				{
					case ir::PTXOperand::pred:
					case ir::PTXOperand::s8:
					case ir::PTXOperand::b8:
					case ir::PTXOperand::u8:
					case ir::PTXOperand::s16:
					case ir::PTXOperand::b16:
					case ir::PTXOperand::u16:
					case ir::PTXOperand::s32:
					case ir::PTXOperand::b32:
					case ir::PTXOperand::u32:
					{
						ir::LLVMBitcast bitcast;
						bitcast.d = _destination( i );
						bitcast.a = _translate( i.a );
	
						_add( bitcast );
						_predicateEpilogue( i, bitcast.d );
						break;
					}
					case ir::PTXOperand::s64:
					case ir::PTXOperand::b64:
					case ir::PTXOperand::u64:
					{
						_translateMov( i );
						break;
					}
					case ir::PTXOperand::f16:
					case ir::PTXOperand::f64:
					case ir::PTXOperand::f32:
					{
						ir::LLVMSitofp sitofp;
						sitofp.d = _destination( i );
						sitofp.a = _translate( i.a );
						
						_add( sitofp );
						_predicateEpilogue( i, sitofp.d );
						break;
					}
					case ir::PTXOperand::TypeSpecifier_invalid:
					{
						assertM( false, "Convert " << i.toString() 
							<< " not implemented." );
						break;
					}
				}
				break;
			}
			case ir::PTXOperand::pred:
			case ir::PTXOperand::b8:
			case ir::PTXOperand::u8:
			{
				switch( i.type )
				{
					case ir::PTXOperand::pred:
					case ir::PTXOperand::s16:
					case ir::PTXOperand::s32:
					case ir::PTXOperand::s64:
					case ir::PTXOperand::b16:
					case ir::PTXOperand::u16:
					case ir::PTXOperand::b32:
					case ir::PTXOperand::u32:
					case ir::PTXOperand::b64:
					case ir::PTXOperand::u64:
					{
						ir::LLVMZext zext;
						zext.d = _destination( i );
						zext.a = _translate( i.a );
						
						_add( zext );
						_predicateEpilogue( i, zext.d );
						break;
					}
					case ir::PTXOperand::s8:
					case ir::PTXOperand::b8:
					case ir::PTXOperand::u8:
					{
						_translateMov( i );
						break;
					}
					case ir::PTXOperand::f16:
					case ir::PTXOperand::f64:
					case ir::PTXOperand::f32:
					{
						ir::LLVMUitofp uitofp;
						uitofp.d = _destination( i );
						uitofp.a = _translate( i.a );
						
						_add( uitofp );
						_predicateEpilogue( i, uitofp.d );
						break;
					}
					case ir::PTXOperand::TypeSpecifier_invalid:
					{
						assertM( false, "Convert " << i.toString() 
							<< " not implemented." );
						break;
					}
				}
				break;
			}
			case ir::PTXOperand::b16:
			case ir::PTXOperand::u16:
			{
				switch( i.type )
				{
					case ir::PTXOperand::b8:
					case ir::PTXOperand::u8:
					case ir::PTXOperand::s8:
					case ir::PTXOperand::pred:
					{
						ir::LLVMBitcast bitcast;
						bitcast.d = _destination( i );
						bitcast.a = _translate( i.a );
	
						_add( bitcast );
						_predicateEpilogue( i, bitcast.d );
						break;
					}
					case ir::PTXOperand::s32:
					case ir::PTXOperand::s64:
					case ir::PTXOperand::b32:
					case ir::PTXOperand::u32:
					case ir::PTXOperand::b64:
					case ir::PTXOperand::u64:
					{
						ir::LLVMZext zext;
						zext.d = _destination( i );
						zext.a = _translate( i.a );
						
						_add( zext );
						_predicateEpilogue( i, zext.d );
						break;
					}
					case ir::PTXOperand::s16:
					case ir::PTXOperand::b16:
					case ir::PTXOperand::u16:
					{
						_translateMov( i );
						break;
					}
					case ir::PTXOperand::f16:
					{
						assertM( false, "F16 type not supported" );
						break;
					}
					case ir::PTXOperand::f64:
					case ir::PTXOperand::f32:
					{
						ir::LLVMUitofp uitofp;
						uitofp.d = _destination( i );
						uitofp.a = _translate( i.a );
						
						_add( uitofp );
						_predicateEpilogue( i, uitofp.d );
						break;
					}
					case ir::PTXOperand::TypeSpecifier_invalid:
					{
						assertM( false, "Convert " << i.toString() 
							<< " not implemented." );
						break;
					}
				}
				break;
			}
			case ir::PTXOperand::b32:
			case ir::PTXOperand::u32:
			{
				switch( i.type )
				{
					case ir::PTXOperand::pred:
					case ir::PTXOperand::b8:
					case ir::PTXOperand::u8:
					case ir::PTXOperand::s8:
					case ir::PTXOperand::b16:
					case ir::PTXOperand::u16:
					case ir::PTXOperand::s16:
					{
						ir::LLVMBitcast bitcast;
						bitcast.d = _destination( i );
						bitcast.a = _translate( i.a );
	
						_add( bitcast );
						_predicateEpilogue( i, bitcast.d );
						break;
					}
					case ir::PTXOperand::b64:
					case ir::PTXOperand::u64:
					case ir::PTXOperand::s64:
					{
						ir::LLVMZext zext;
						zext.d = _destination( i );
						zext.a = _translate( i.a );
						
						_add( zext );
						_predicateEpilogue( i, zext.d );
						break;
					}
					case ir::PTXOperand::b32:
					case ir::PTXOperand::s32:
					case ir::PTXOperand::u32:
					{
						_translateMov( i );
						break;
					}
					case ir::PTXOperand::f16:
					{
						assertM( false, "F16 type not supported" );
						break;
					}
					case ir::PTXOperand::f64:
					case ir::PTXOperand::f32:
					{
						ir::LLVMUitofp uitofp;
						uitofp.d = _destination( i );
						uitofp.a = _translate( i.a );
						
						_add( uitofp );
						_predicateEpilogue( i, uitofp.d );
						break;
					}
					case ir::PTXOperand::TypeSpecifier_invalid:
					{
						assertM( false, "Convert " << i.toString() 
							<< " not implemented." );
						break;
					}
				}
				break;
			}
			case ir::PTXOperand::b64:
			case ir::PTXOperand::u64:
			{
				switch( i.type )
				{
					case ir::PTXOperand::pred:
					case ir::PTXOperand::s8:
					case ir::PTXOperand::b8:
					case ir::PTXOperand::u8:
					case ir::PTXOperand::b16:
					case ir::PTXOperand::u16:
					case ir::PTXOperand::s16:
					case ir::PTXOperand::b32:
					case ir::PTXOperand::u32:
					case ir::PTXOperand::s32:
					{
						ir::LLVMBitcast bitcast;
						bitcast.d = _destination( i );
						bitcast.a = _translate( i.a );
	
						_add( bitcast );
						_predicateEpilogue( i, bitcast.d );
						break;
					}
					case ir::PTXOperand::b64:
					case ir::PTXOperand::s64:
					case ir::PTXOperand::u64:
					{
						_translateMov( i );
						break;
					}
					case ir::PTXOperand::f16:
					{
						assertM( false, "F16 type not supported" );
						break;
					}
					case ir::PTXOperand::f64:
					case ir::PTXOperand::f32:
					{
						ir::LLVMUitofp uitofp;
						uitofp.d = _destination( i );
						uitofp.a = _translate( i.a );
						
						_add( uitofp );
						_predicateEpilogue( i, uitofp.d );
						break;
					}
					case ir::PTXOperand::TypeSpecifier_invalid:
					{
						assertM( false, "Convert " << i.toString() 
							<< " not implemented." );
						break;
					}
				}
				break;
			}
			case ir::PTXOperand::f16:
			{
				switch( i.type )
				{
					case ir::PTXOperand::s8:
					case ir::PTXOperand::s16:
					case ir::PTXOperand::s32:
					case ir::PTXOperand::s64:
					{
						ir::LLVMFptosi fptosi;
						fptosi.d = _destination( i );
						fptosi.a = _translate( i.a );
						
						_add( fptosi );
						_predicateEpilogue( i, fptosi.d );
						break;
					}
					case ir::PTXOperand::pred:
					case ir::PTXOperand::b8:
					case ir::PTXOperand::u8:
					case ir::PTXOperand::b16:
					case ir::PTXOperand::u16:
					case ir::PTXOperand::b32:
					case ir::PTXOperand::u32:
					case ir::PTXOperand::b64:
					case ir::PTXOperand::u64:
					{
						ir::LLVMFptoui fptoui;
						fptoui.d = _destination( i );
						fptoui.a = _translate( i.a );
						
						_add( fptoui );
						_predicateEpilogue( i, fptoui.d );
						break;
					}
					case ir::PTXOperand::f16:
					{
						_translateMov( i );
						break;
					}
					case ir::PTXOperand::f32:
					case ir::PTXOperand::f64:
					{
						ir::LLVMFpext fpext;
						fpext.d = _destination( i );
						fpext.a = _translate( i.a );
						
						_add( fpext );
						_predicateEpilogue( i, fpext.d );
						break;
					}
					case ir::PTXOperand::TypeSpecifier_invalid:
					{
						assertM( false, "Convert " << i.toString() 
							<< " not implemented." );
						break;
					}
				}
				break;
			}
			case ir::PTXOperand::f32:
			{
				switch( i.type )
				{
					case ir::PTXOperand::s8:
					case ir::PTXOperand::s16:
					case ir::PTXOperand::s32:
					case ir::PTXOperand::s64:
					{
						ir::LLVMFptosi fptosi;
						fptosi.d = _destination( i );
						fptosi.a = _translate( i.a );
						
						_add( fptosi );
						_predicateEpilogue( i, fptosi.d );
						break;
					}
					case ir::PTXOperand::pred:
					case ir::PTXOperand::b8:
					case ir::PTXOperand::u8:
					case ir::PTXOperand::b16:
					case ir::PTXOperand::u16:
					case ir::PTXOperand::b32:
					case ir::PTXOperand::u32:
					case ir::PTXOperand::b64:
					case ir::PTXOperand::u64:
					{
						ir::LLVMFptoui fptoui;
						fptoui.d = _destination( i );
						fptoui.a = _translate( i.a );
						
						_add( fptoui );
						_predicateEpilogue( i, fptoui.d );
						break;
					}
					case ir::PTXOperand::f16:
					{
						ir::LLVMFptrunc fptrunc;
						fptrunc.d = _destination( i );
						fptrunc.a = _translate( i.a );
						
						_add( fptrunc );
						_predicateEpilogue( i, fptrunc.d );
						break;
					}
					case ir::PTXOperand::f32:
					{
						_translateMov( i );
						break;
					}
					case ir::PTXOperand::f64:
					{
						ir::LLVMFpext fpext;
						fpext.d = _destination( i );
						fpext.a = _translate( i.a );
						
						_add( fpext );
						_predicateEpilogue( i, fpext.d );
						break;
					}
					case ir::PTXOperand::TypeSpecifier_invalid:
					{
						assertM( false, "Convert " << i.toString() 
							<< " not implemented." );
						break;
					}
				}
				break;
			}
			case ir::PTXOperand::f64:
			{
				switch( i.type )
				{
					case ir::PTXOperand::s8:
					case ir::PTXOperand::s16:
					case ir::PTXOperand::s32:
					case ir::PTXOperand::s64:
					{
						ir::LLVMFptosi fptosi;
						fptosi.d = _destination( i );
						fptosi.a = _translate( i.a );
						
						_add( fptosi );
						_predicateEpilogue( i, fptosi.d );
						break;
					}
					case ir::PTXOperand::pred:
					case ir::PTXOperand::b8:
					case ir::PTXOperand::u8:
					case ir::PTXOperand::b16:
					case ir::PTXOperand::u16:
					case ir::PTXOperand::b32:
					case ir::PTXOperand::u32:
					case ir::PTXOperand::b64:
					case ir::PTXOperand::u64:
					{
						ir::LLVMFptoui fptoui;
						fptoui.d = _destination( i );
						fptoui.a = _translate( i.a );
						
						_add( fptoui );
						_predicateEpilogue( i, fptoui.d );
						break;
					}
					case ir::PTXOperand::f16:
					{
						ir::LLVMFptrunc fptrunc;
						fptrunc.d = _destination( i );
						fptrunc.a = _translate( i.a );
						
						_add( fptrunc );
						_predicateEpilogue( i, fptrunc.d );
						break;
					}
					case ir::PTXOperand::f32:
					{
						ir::LLVMFptrunc fptrunc;
						fptrunc.d = _destination( i );
						fptrunc.a = _translate( i.a );
						
						_add( fptrunc );
						_predicateEpilogue( i, fptrunc.d );
						break;
					}
					case ir::PTXOperand::f64:
					{
						_translateMov( i );
						break;
					}
					case ir::PTXOperand::TypeSpecifier_invalid:
					{
						assertM( false, "Convert " << i.toString() 
							<< " not implemented." );
						break;
					}
				}
				break;
			}
			case ir::PTXOperand::TypeSpecifier_invalid:
			{
				assertM( false, "Convert " << i.toString() 
					<< " not implemented." );
				break;
			}
		}
	}

	void PTXToLLVMTranslator::_translateDiv( const ir::PTXInstruction& i )
	{
		if( ir::PTXOperand::isFloat( i.type ) )
		{
			ir::LLVMFdiv div;
			
			div.d = _destination( i );
			div.a = _translate( i.a );
			div.b = _translate( i.b );
			
			_add( div );
			_predicateEpilogue( i, div.d );		
		}
		else if( ir::PTXOperand::isSigned( i.type ) )
		{
			ir::LLVMSdiv div;
			
			div.d = _destination( i );
			div.a = _translate( i.a );
			div.b = _translate( i.b );
			
			_add( div );
			_predicateEpilogue( i, div.d );		
		}
		else
		{
			ir::LLVMUdiv div;
			
			div.d = _destination( i );
			div.a = _translate( i.a );
			div.b = _translate( i.b );
			
			_add( div );
			_predicateEpilogue( i, div.d );				
		}
	}

	void PTXToLLVMTranslator::_translateEx2( const ir::PTXInstruction& i )
	{
		ir::LLVMCall call;
		
		if( i.modifier & ir::PTXInstruction::ftz )
		{
			call.name = "translator::ex2Ftz";
		}
		else
		{
			call.name = "translator::ex2";
		}
		
		call.d = _destination( i );
		call.parameters.resize( 1 );
		call.parameters[0] = _translate( i.a );
		
		_add( call );
		_predicateEpilogue( i, call.d );
	}

	void PTXToLLVMTranslator::_translateExit( const ir::PTXInstruction& i )
	{
		_yield();
	}

	void PTXToLLVMTranslator::_translateLd( const ir::PTXInstruction& i )
	{
		ir::LLVMLoad load;
		
		load.d = _destination( i );

		ir::LLVMInttoptr inttoptr;
		
		if( i.a.offset != 0 )
		{
			ir::LLVMAdd add;
		
			add.a = _translate( i.a );
			add.b = add.a;
			add.b.constant = true;
			add.b.i64 = i.a.offset;
			add.d = add.a;
			add.d.name = _tempRegister();
	
			_add( add );
			
			inttoptr.a = add.d;
			inttoptr.d = load.d;
			inttoptr.d.type.category = ir::LLVMInstruction::Type::Pointer;
			inttoptr.d.name = _tempRegister();
		
			_add( inttoptr );
		}
		else
		{
			inttoptr.a = _translate( i.a );
			inttoptr.d = load.d;
			inttoptr.d.type.category = ir::LLVMInstruction::Type::Pointer;
			inttoptr.d.name = _tempRegister();
		
			_add( inttoptr );
		}		
				
		load.a = inttoptr.d;
		
		if( i.volatility == ir::PTXInstruction::Volatile )
		{
			load.isVolatile = true;
		}
		
		load.alignment = i.d.bytes();
		
		_add( load );
		_predicateEpilogue( i, load.d );
	}

	void PTXToLLVMTranslator::_translateLg2( const ir::PTXInstruction& i )
	{
		ir::LLVMCall call;
		
		if( i.modifier & ir::PTXInstruction::ftz )
		{
			call.name = "translator::lg2Ftz";
		}
		else
		{
			call.name = "translator::lg2";
		}
		
		call.d = _destination( i );
		call.parameters.resize( 1 );
		call.parameters[0] = _translate( i.a );
		
		_add( call );
		_predicateEpilogue( i, call.d );
	}

	void PTXToLLVMTranslator::_translateMad24( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateMad( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateMax( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateMembar( const ir::PTXInstruction& i )
	{
		ir::LLVMCall call;
		
		if( i.level == ir::PTXInstruction::CtaLevel )
		{
			call.name = "translator::membarCta";
		}
		else
		{
			call.name = "translator::membarGlobal";
		}
		
		call.d = _destination( i );
		
		_add( call );
		_predicateEpilogue( i, call.d );
	}

	void PTXToLLVMTranslator::_translateMin( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateMov( const ir::PTXInstruction& i )
	{
		if( i.d.bytes() == i.a.bytes() )
		{
			ir::LLVMSelect select;
			select.d = _destination( i );
			select.a = _translate( i.a );
			select.b = select.a;
			select.condition.type.category = ir::LLVMInstruction::Type::Element;
			select.condition.type.type = ir::LLVMInstruction::I1;
			select.condition.constant = true;
			select.condition.i1 = true;
		
			_add( select );
			_predicateEpilogue( i, select.d );
		}
		else
		{
			_translateCvt( i );
		}
	}

	void PTXToLLVMTranslator::_translateMul24( const ir::PTXInstruction& i )
	{
		if( i.modifier & ir::PTXInstruction::lo )
		{
			// 24-bit lo is the same as 32-bit lo
			ir::LLVMMul mul;
					
			mul.d = _destination( i );
			mul.a = _translate( i.a );
			mul.b = _translate( i.b );
		
			_add( mul );
			_predicateEpilogue( i, mul.d );		
		}
		else
		{
			assertM( false, "No support for hi multiply" );
		}	
	}

	void PTXToLLVMTranslator::_translateMul( const ir::PTXInstruction& i )
	{
		if( ir::PTXOperand::isFloat( i.type ) )
		{
			ir::LLVMFmul mul;
			
			_setFloatingPointRoundingMode( i );
			
			mul.d = _destination( i );
			mul.a = _translate( i.a );
			mul.b = _translate( i.b );

			if( i.modifier & ir::PTXInstruction::sat )
			{
				assertM( false, "Saturate not supported for " << i.toString() );
			}

			_add( mul );
			_predicateEpilogue( i, mul.d );
		}
		else
		{
			if( i.modifier & ir::PTXInstruction::wide )
			{
				ir::LLVMSext sextA;
				sextA.a = _translate( i.a );
				sextA.d = sextA.a;
				_doubleWidth( sextA.d.type.type );
				sextA.d.name = _tempRegister();
				
				_add( sextA );

				ir::LLVMSext sextB;
				sextB.a = _translate( i.b );
				sextB.d = sextB.a;
				_doubleWidth( sextB.d.type.type );
				sextB.d.name = _tempRegister();
				
				_add( sextB );

				ir::LLVMMul mul;
						
				mul.d = _destination( i );
				mul.a = sextA.d;
				mul.b = sextB.d;
			
				_add( mul );
				_predicateEpilogue( i, mul.d );
			}
			else if( i.modifier & ir::PTXInstruction::lo )
			{
				ir::LLVMMul mul;
						
				mul.d = _destination( i );
				mul.a = _translate( i.a );
				mul.b = _translate( i.b );
			
				_add( mul );
				_predicateEpilogue( i, mul.d );		
			}
			else
			{
				assertM( false, "No support for hi multiply" );
			}
		}
	}

	void PTXToLLVMTranslator::_translateNeg( const ir::PTXInstruction& i )
	{
		ir::LLVMSub sub;
		
		sub.d = _destination( i );
		sub.b = _translate( i.a );
		sub.a = sub.b;
		sub.a.constant = true;
		sub.a.i64 = 0;
		
		_add( sub );
		_predicateEpilogue( i, sub.d );
	}

	void PTXToLLVMTranslator::_translateNot( const ir::PTXInstruction& i )
	{
		ir::LLVMXor Not;
		
		Not.d = _destination( i );
		Not.a = _translate( i.a );
		Not.b = Not.a;
		Not.b.constant = true;
		Not.b.i64 = -1;
		
		_add( Not );
		_predicateEpilogue( i, Not.d );
	}

	void PTXToLLVMTranslator::_translateOr( const ir::PTXInstruction& i )
	{
		ir::LLVMOr Or;
		
		Or.d = _destination( i );
		Or.a = _translate( i.a );
		Or.b = _translate( i.b );

		_add( Or );
		_predicateEpilogue( i, Or.d );
	}

	void PTXToLLVMTranslator::_translatePmevent( const ir::PTXInstruction& i )
	{
		ir::LLVMCall call;
		
		call.name = "translator::pmevent";
		
		call.parameters.resize( 1 );
		call.parameters[0] = _translate( i.a );
		
		_add( call );
	}

	void PTXToLLVMTranslator::_translateRcp( const ir::PTXInstruction& i )
	{
		ir::LLVMFdiv div;
		
		div.d = _destination( i );
		div.b = _translate( i.a );
		div.a = div.b;
		div.a.constant = true;
		
		if( i.a.type == ir::PTXOperand::f32 )
		{
			div.a.f32 = 1.0;
		}
		else
		{
			div.a.f64 = 1.0;
		}
		
		_add( div );
		_predicateEpilogue( i, div.d );
	}

	void PTXToLLVMTranslator::_translateRed( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateRem( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateRet( const ir::PTXInstruction& i )
	{
		_yield();
	}

	void PTXToLLVMTranslator::_translateRsqrt( const ir::PTXInstruction& i )
	{
		ir::LLVMCall call;
		
		if( i.modifier & ir::PTXInstruction::ftz )
		{
			call.name = "translator::rsqrtFtz";
		}
		else
		{
			call.name = "translator::rsqrt";
		}
		
		call.d = _destination( i );
		call.parameters.resize( 1 );
		call.parameters[0] = _translate( i.a );
		
		_add( call );
		_predicateEpilogue( i, call.d );
	}

	void PTXToLLVMTranslator::_translateSad( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateSelP( const ir::PTXInstruction& i )
	{
		ir::LLVMSelect select;
		
		select.d = _destination( i );
		select.a = _translate( i.a );
		select.b = _translate( i.b );
		select.condition = _translate( i.c );
		
		_add( select );
		_predicateEpilogue( i, select.d );
	}

	void PTXToLLVMTranslator::_translateSet( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateSetP( const ir::PTXInstruction& i )
	{
		ir::LLVMInstruction::Operand d = _destination( i );
		ir::LLVMInstruction::Operand tempD;

		if( i.c.addressMode == ir::PTXOperand::Register )
		{
			tempD = _destination( i, true );
		}
		else
		{
			tempD = d;
		}

		if( ir::PTXOperand::isFloat( i.type ) )
		{
			ir::LLVMFcmp fcmp;
					
			fcmp.d = tempD;
			fcmp.a = _translate( i.a );
			fcmp.b = _translate( i.b );
			fcmp.comparison = _translate( i.comparisonOperator, false );
			
			_add( fcmp );
		}
		else
		{
			ir::LLVMIcmp icmp;
					
			icmp.d = tempD;
			icmp.a = _translate( i.a );
			icmp.b = _translate( i.b );
			icmp.comparison = _translate( i.comparisonOperator, false );
			
			_add( icmp );		
		}
		
		ir::LLVMInstruction::Operand pd = d;
		ir::LLVMInstruction::Operand pq;
		
		if( i.pq.addressMode != ir::PTXOperand::Invalid )
		{
			pq = _translate( i.pq );
		}
		
		if( i.c.addressMode == ir::PTXOperand::Register )
		{
			ir::LLVMXor Not;

			if( i.pq.addressMode != ir::PTXOperand::Invalid )
			{
				Not.d = tempD;
				Not.d.name = _tempRegister();
				Not.a = tempD;
				Not.b.type.category = ir::LLVMInstruction::Type::Element;
				Not.b.type.type = ir::LLVMInstruction::I1;
				Not.b.constant = true;
				Not.b.i1 = true;
			
				_add( Not );
			}
					
			switch( i.booleanOperator )
			{
				case ir::PTXInstruction::BoolAnd:
				{
					ir::LLVMAnd And;
					
					And.d = pd;
					And.a = _translate( i.c );
					And.b = tempD;
					
					_add( And );
					
					if( i.pq.addressMode != ir::PTXOperand::Invalid )
					{
						And.d = pq;
						And.b = Not.d;
					
						_add( And );
					}
					break;
				}
				case ir::PTXInstruction::BoolOr:
				{
					ir::LLVMOr Or;
					
					Or.d = pd;
					Or.a = _translate( i.c );
					Or.b = tempD;
					
					_add( Or );
					
					if( i.pq.addressMode != ir::PTXOperand::Invalid )
					{
						Or.d = pq;
						Or.b = Not.d;
					
						_add( Or );
					}
					break;
				}
				case ir::PTXInstruction::BoolXor:
				{
					ir::LLVMXor Xor;
					
					Xor.d = pd;
					Xor.a = _translate( i.c );
					Xor.b = tempD;
					
					_add( Xor );

					if( i.pq.addressMode != ir::PTXOperand::Invalid )
					{
						Xor.d = pq;
						Xor.b = Not.d;
					
						_add( Xor );
					}
					break;
				}
				default:
				{
					break;
				}
			}
		}
		
		_predicateEpilogue( i, d );
		if( i.pq.addressMode != ir::PTXOperand::Invalid )
		{
			_predicateEpilogue( i, tempD );
		}
	}

	void PTXToLLVMTranslator::_translateShl( const ir::PTXInstruction& i )
	{
		ir::LLVMShl shift;
		
		shift.d = _destination( i );
		shift.a = _translate( i.a );
		shift.b = _translate( i.b );
		
		_add( shift );
		_predicateEpilogue( i, shift.d );
	}

	void PTXToLLVMTranslator::_translateShr( const ir::PTXInstruction& i )
	{
		if( ir::PTXOperand::isSigned( i.type ) )
		{
			ir::LLVMLshr shift;
			
			shift.d = _destination( i );
			shift.a = _translate( i.a );
			shift.b = _translate( i.b );
			
			_add( shift );
			_predicateEpilogue( i, shift.d );
		}
		else
		{
			ir::LLVMAshr shift;
			
			shift.d = _destination( i );
			shift.a = _translate( i.a );
			shift.b = _translate( i.b );
			
			_add( shift );
			_predicateEpilogue( i, shift.d );		
		}
	}

	void PTXToLLVMTranslator::_translateSin( const ir::PTXInstruction& i )
	{
		ir::LLVMCall call;
		
		if( i.modifier & ir::PTXInstruction::ftz )
		{
			call.name = "translator::sinFtz";
		}
		else
		{
			call.name = "translator::sin";
		}
		
		call.d = _destination( i );
		call.parameters.resize( 1 );
		call.parameters[0] = _translate( i.a );
		
		_add( call );
		_predicateEpilogue( i, call.d );
	}

	void PTXToLLVMTranslator::_translateSlCt( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateSqrt( const ir::PTXInstruction& i )
	{
		ir::LLVMCall call;
		
		if( i.modifier & ir::PTXInstruction::ftz )
		{
			call.name = "translator::sqrtFtz";
		}
		else
		{
			call.name = "translator::sqrt";
		}
		
		call.d = _destination( i );
		call.parameters.resize( 1 );
		call.parameters[0] = _translate( i.a );
		
		_add( call );
		_predicateEpilogue( i, call.d );
	}

	void PTXToLLVMTranslator::_translateSt( const ir::PTXInstruction& i )
	{
		assertM( i.pg.condition == ir::PTXOperand::PT, 
			"Predicated store instructions not supported." );

		ir::LLVMStore store;
		
		store.a = _translate( i.a );
		store.d = _translate( i.d );

		ir::LLVMInttoptr inttoptr;
		
		if( i.d.offset != 0 )
		{
			ir::LLVMAdd add;
		
			add.a = store.d;
			add.b = add.a;
			add.b.constant = true;
			add.b.i64 = i.d.offset;
			add.d = add.a;
			add.d.name = _tempRegister();
	
			_add( add );
			
			inttoptr.a = add.d;
			inttoptr.d = store.a;
			inttoptr.d.type.category = ir::LLVMInstruction::Type::Pointer;
			inttoptr.d.name = _tempRegister();		
		}
		else
		{
			inttoptr.a = store.d;
			inttoptr.d = store.a;
			inttoptr.d.type.category = ir::LLVMInstruction::Type::Pointer;
			inttoptr.d.name = _tempRegister();
		}		

		_add( inttoptr );
						
		if( i.volatility == ir::PTXInstruction::Volatile )
		{
			store.isVolatile = true;
		}
		
		store.d = inttoptr.d;
		store.alignment = i.a.bytes();
		
		_add( store );
		_predicateEpilogue( i, store.d );
	}

	void PTXToLLVMTranslator::_translateSub( const ir::PTXInstruction& i )
	{
		if( ir::PTXOperand::isFloat( i.type ) )
		{
			ir::LLVMFsub sub;
		
			sub.d = _destination( i );
			sub.a = _translate( i.a );
			sub.b = _translate( i.b );
		
			_add( sub );
			
			if( i.modifier & ir::PTXInstruction::sat )
			{
				ir::LLVMFcmp compare;
				
				compare.d.name = _tempRegister();
				compare.d.type.type = ir::LLVMInstruction::I1;
				compare.d.type.category = ir::LLVMInstruction::Type::Element;
				compare.comparison = ir::LLVMInstruction::Ule;
				compare.a = sub.d;
				compare.b.type.type = compare.a.type.type;
				compare.b.type.category = ir::LLVMInstruction::Type::Element;
				compare.b.constant = true;
				if( compare.b.type.type == ir::LLVMInstruction::F32 )
				{
					compare.b.f32 = 0;
				}
				else
				{
					compare.b.f64 = 0;
				}
				
				ir::LLVMSelect select;
				
				select.d.name = _tempRegister();
				select.d.type.type = sub.d.type.type;
				select.d.type.category = sub.d.type.category;
				select.condition = compare.d;
				select.a = compare.b;
				select.b = sub.d;
				
				_add( compare );
				_add( select );
				_predicateEpilogue( i, select.d );
			}
			else
			{
				_predicateEpilogue( i, sub.d );
			}
		}
		else
		{
			assertM( !(i.modifier & ir::PTXInstruction::sat), 
				"Saturation for ptx int sub not supported." );
		
			ir::LLVMSub sub;
			
			sub.d = _destination( i );
			sub.a = _translate( i.a );
			sub.b = _translate( i.b );
		
			_add( sub );
			_predicateEpilogue( i, sub.d );
		}
	}

	void PTXToLLVMTranslator::_translateSubC( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateTex( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateTrap( const ir::PTXInstruction& i )
	{
		ir::LLVMCall call;
		
		call.name = "translator::trap";
		
		_add( call );
		_predicateEpilogue( i, call.d );
	}

	void PTXToLLVMTranslator::_translateVote( const ir::PTXInstruction& i )
	{
		assertM( false, "Opcode " 
			<< ir::PTXInstruction::toString( i.opcode ) 
			<< " not supported." );
	}

	void PTXToLLVMTranslator::_translateXor( const ir::PTXInstruction& i )
	{
		ir::LLVMXor Xor;
		
		Xor.d = _destination( i );
		Xor.a = _translate( i.a );
		Xor.b = _translate( i.b );

		_add( Xor );
		_predicateEpilogue( i, Xor.d );
	}

	std::string PTXToLLVMTranslator::_tempRegister()
	{
		std::stringstream stream;
		stream << "$rt" << _tempRegisterCount++;
		return stream.str();
	}

	void PTXToLLVMTranslator::_setFloatingPointRoundingMode( 
		const ir::PTXInstruction& i )
	{
		if( i.modifier & ir::PTXInstruction::rn )
		{
			assertM( false, "Rn rounding mode not implemented" );
		}
		else if( i.modifier & ir::PTXInstruction::rz )
		{
			assertM( false, "Rz rounding mode not implemented" );
		}
		else if( i.modifier & ir::PTXInstruction::rm )
		{
			assertM( false, "Rm rounding mode not implemented" );
		}
		else if( i.modifier & ir::PTXInstruction::rp )
		{
			assertM( false, "Rp rounding mode not implemented" );
		}
		else if( i.modifier & ir::PTXInstruction::rz )
		{
			assertM( false, "Rz rounding mode not implemented" );
		}
	}

	ir::LLVMInstruction::Operand PTXToLLVMTranslator::_destination( 
		const ir::PTXInstruction& i, bool pq )
	{
		assertM( _producers.count( i.d.reg ) == 0, 
			"PTX program is not in ssa form, register " << i.d.reg 
			<< " assigned twice." );
		_producers.insert( std::make_pair( i.d.reg, _instructionId ) );
		ir::LLVMInstruction::Operand destination;
		if( pq )
		{
			destination = _translate( i.pq );
		}
		else
		{
			destination = _translate( i.d );
		}
		if( i.pg.condition != ir::PTXOperand::PT )
		{
			destination.name = _tempRegister();			
		}
		return destination;
	}
	
	void PTXToLLVMTranslator::_predicateEpilogue( const ir::PTXInstruction& i,
		const ir::LLVMInstruction::Operand& temp )
	{
		if( i.pg.condition == ir::PTXOperand::PT ) return;
		
		ir::LLVMSelect select;
		select.condition = _translate( i.pg );
		select.d = _translate( i.d );
	
		if( i.pg.condition == ir::PTXOperand::nPT )
		{
			select.a = select.d;
			select.b = select.d;
		}
		else if( i.pg.condition == ir::PTXOperand::Pred )
		{
			select.a = temp;
			select.b = select.d;
		}
		else
		{
			select.a = select.d;
			select.b = temp;
		}
		
		_add( select );
	}

	void PTXToLLVMTranslator::_add( const ir::LLVMInstruction& i )
	{
		assertM( i.valid() == "", "Instruction " << i.toString() 
			<< " is not valid " << i.valid() );
		report( "    Added instruction " << i.toString() );
		_llvmKernel->_statements.push_back( ir::LLVMStatement( i ) );	
	}

	void PTXToLLVMTranslator::_initializePhiInstructions()
	{
		typedef std::queue< ir::Instruction::RegisterType > RegisterQueue;
		
		for( IndexVector::const_iterator pi = _phiIndices.begin(); 
			pi != _phiIndices.end(); ++pi )
		{
			RegisterQueue bfs;
			
			ir::LLVMPhi& phi = static_cast< ir::LLVMPhi& >( 
				*_llvmKernel->_statements[ *pi ].instruction );
			
			for( ir::LLVMPhi::NodeVector::const_iterator 
				node = phi.nodes.begin(); node != phi.nodes.end(); ++node )
			{
				bfs.push( node->reg );
			}
			
			while( !bfs.empty() )
			{
				RegisterToIndexMap::const_iterator 
					producer = _producers.find( bfs.front() );
				if( producer != _producers.end() )
				{
					assert( producer->second 
						< _llvmKernel->instructions.size() );
					_swapAllExceptName( phi.d, 
						_llvmKernel->instructions[ producer->second ].d );
					for( ir::LLVMPhi::NodeVector::iterator 
						node = phi.nodes.begin(); 
						node != phi.nodes.end(); ++node )
					{
						_swapAllExceptName( node->operand, 
							_llvmKernel->instructions[ producer->second ].d );
					}
					break;
				}
				else
				{
					RegisterToIndexMap::const_iterator 
						pi = _phiProducers.find( bfs.front() );
					assertM( pi != _phiProducers.end(), "Phi source register " 
						<< bfs.front() 
						<< " not destination of any phi or ptx instruction." );
					const ir::LLVMPhi& phi = static_cast< const ir::LLVMPhi& >( 
						*_llvmKernel->_statements[ pi->second ].instruction );
					for( ir::LLVMPhi::NodeVector::const_iterator 
						node = phi.nodes.begin(); node != phi.nodes.end(); 
						++node )
					{
						bfs.push( node->reg );
					}			
				}
				bfs.pop();
			}
		}
	}

	PTXToLLVMTranslator::PTXToLLVMTranslator( OptimizationLevel l ) 
		: Translator( ir::Instruction::PTX, ir::Instruction::LLVM, l ),
		_tempRegisterCount( 0 ), _tempBlockCount( 0 )
	{
	
	}
	
	PTXToLLVMTranslator::~PTXToLLVMTranslator()
	{
	
	}
	
	ir::Kernel* PTXToLLVMTranslator::translate( const ir::Kernel* k )
	{
		report( "Translating PTX kernel " << k->name );
		_llvmKernel = new ir::LLVMKernel( *k );	
		
		_convertPtxToSsa();
		_translateInstructions();
		_initializePhiInstructions();
		
		_tempRegisterCount = 0;
		_producers.clear();
		_phiProducers.clear();
		_phiIndices.clear();
		delete _graph;
		
		return _llvmKernel;
	}
	
	void PTXToLLVMTranslator::addProfile( const ProfilingData& d )
	{
		// profiling not yet implemented
	}
}

#endif
