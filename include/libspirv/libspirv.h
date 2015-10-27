// Copyright (c) 2015 The Khronos Group Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and/or associated documentation files (the
// "Materials"), to deal in the Materials without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Materials, and to
// permit persons to whom the Materials are furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Materials.
//
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/
//
// THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

#ifndef _CODEPLAY_SPIRV_SPIRV_H_
#define _CODEPLAY_SPIRV_SPIRV_H_

#include <headers/spirv.hpp>
#include <headers/spirv_operands.hpp>
#include <headers/GLSL.std.450.h>
#include <headers/OpenCL.std.h>

#ifdef __cplusplus
using namespace spv;
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

// Magic numbers

#define SPV_MAGIC_NUMBER 0x07230203
#define SPV_VERSION_NUMBER 99u

// Header indices

#define SPV_INDEX_MAGIC_NUMBER 0u
#define SPV_INDEX_VERSION_NUMBER 1u
#define SPV_INDEX_GENERATOR_NUMBER 2u
#define SPV_INDEX_BOUND 3u
#define SPV_INDEX_SCHEMA 4u
#define SPV_INDEX_INSTRUCTION 5u

// Universal limits

// SPIR-V 1.0 limits
#define SPV_LIMIT_INSTRUCTION_WORD_COUNT_MAX 0xffff
#define SPV_LIMIT_LITERAL_STRING_UTF8_CHARS_MAX 0xffff

// A single Unicode character in UTF-8 encoding can take
// up 4 bytes.
#define SPV_LIMIT_LITERAL_STRING_BYTES_MAX \
  (SPV_LIMIT_LITERAL_STRING_UTF8_CHARS_MAX * 4)

// NOTE: These are set to the minimum maximum values
// TODO(dneto): Check these.

// libspirv limits.
#define SPV_LIMIT_RESULT_ID_BOUND 0x00400000
#define SPV_LIMIT_CONTROL_FLOW_NEST_DEPTH 0x00000400
#define SPV_LIMIT_GLOBAL_VARIABLES_MAX 0x00010000
#define SPV_LIMIT_LOCAL_VARIABLES_MAX 0x00080000
// TODO: Decorations per target ID max, depends on decoration table size
#define SPV_LIMIT_EXECUTION_MODE_PER_ENTRY_POINT_MAX 0x00000100
#define SPV_LIMIT_INDICIES_MAX_ACCESS_CHAIN_COMPOSITE_MAX 0x00000100
#define SPV_LIMIT_FUNCTION_PARAMETERS_PER_FUNCTION_DECL 0x00000100
#define SPV_LIMIT_FUNCTION_CALL_ARGUMENTS_MAX 0x00000100
#define SPV_LIMIT_EXT_FUNCTION_CALL_ARGUMENTS_MAX 0x00000100
#define SPV_LIMIT_SWITCH_LITERAL_LABEL_PAIRS_MAX 0x00004000
#define SPV_LIMIT_STRUCT_MEMBERS_MAX 0x0000400
#define SPV_LIMIT_STRUCT_NESTING_DEPTH_MAX 0x00000100

// Helpers

#define spvIsInBitfield(value, bitfield) (value == (value & bitfield))

#define SPV_BIT(shift) (1 << (shift))

#define SPV_FORCE_16_BIT_ENUM(name) _##name = 0x7fff
#define SPV_FORCE_32_BIT_ENUM(name) _##name = 0x7fffffff

#define SPV_OPERAND_INVALID_RESULT_ID_INDEX -1

// A bit mask representing a set of capabilities.
// Currently there are 54 distinct capabilities, so 64 bits
// should be enough.
using spv_capability_mask_t = uint64_t;

// Transforms spv::Capability into a mask for use in bitfields.  Should really
// be a constexpr inline function, but some important versions of MSVC don't
// support that yet.  Different from SPV_BIT, which doesn't guarantee 64-bit
// values.
#define SPV_CAPABILITY_AS_MASK(capability) (spv_capability_mask_t(1) << (capability))

// Enumerations

typedef enum spv_generator_t {
  SPV_GENERATOR_KHRONOS = 0,
  SPV_GENERATOR_VALVE = 1,
  SPV_GENERATOR_LUNARG = 2,
  SPV_GENERATOR_CODEPLAY = 3,
  SPV_FORCE_32_BIT_ENUM(spv_generator_t)
} spv_generator_t;

typedef enum spv_result_t {
  SPV_SUCCESS = 0,
  SPV_UNSUPPORTED = 1,
  SPV_END_OF_STREAM = 2,
  SPV_WARNING = 3,
  SPV_FAILED_MATCH = 4,
  SPV_ERROR_INTERNAL = -1,
  SPV_ERROR_OUT_OF_MEMORY = -2,
  SPV_ERROR_INVALID_POINTER = -3,
  SPV_ERROR_INVALID_BINARY = -4,
  SPV_ERROR_INVALID_TEXT = -5,
  SPV_ERROR_INVALID_TABLE = -6,
  SPV_ERROR_INVALID_VALUE = -7,
  SPV_ERROR_INVALID_DIAGNOSTIC = -8,
  SPV_ERROR_INVALID_LOOKUP = -9,
  SPV_ERROR_INVALID_ID = -10,
  SPV_FORCE_32_BIT_ENUM(spv_result_t)
} spv_result_t;

typedef enum spv_endianness_t {
  SPV_ENDIANNESS_LITTLE,
  SPV_ENDIANNESS_BIG,
  SPV_FORCE_32_BIT_ENUM(spv_endianness_t)
} spv_endianness_t;

// The kinds of operands that an instruction may have.
//
// In addition to determining what kind of value an operand may be, certain
// enums capture the fact that an operand might be optional (may be absent,
// or present exactly once), or might occure zero or more times.
//
// Sometimes we also need to be able to express the fact that an operand
// is a member of an optional tuple of values.  In that case the first member
// would be optional, and the subsequent members would be required.
typedef enum spv_operand_type_t {
  SPV_OPERAND_TYPE_NONE = 0,
  SPV_OPERAND_TYPE_ID,
  SPV_OPERAND_TYPE_TYPE_ID,
  SPV_OPERAND_TYPE_RESULT_ID,
  SPV_OPERAND_TYPE_LITERAL_INTEGER,
  // A literal number that can (but is not required to) expand multiple words.
  SPV_OPERAND_TYPE_MULTIWORD_LITERAL_NUMBER,
  SPV_OPERAND_TYPE_LITERAL_STRING,
  SPV_OPERAND_TYPE_SOURCE_LANGUAGE,
  SPV_OPERAND_TYPE_EXECUTION_MODEL,
  SPV_OPERAND_TYPE_ADDRESSING_MODEL,
  SPV_OPERAND_TYPE_MEMORY_MODEL,
  SPV_OPERAND_TYPE_EXECUTION_MODE,
  SPV_OPERAND_TYPE_STORAGE_CLASS,
  SPV_OPERAND_TYPE_DIMENSIONALITY,
  SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE,
  SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE,
  SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT,
  SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER,
  SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE,
  SPV_OPERAND_TYPE_FP_FAST_MATH_MODE,
  SPV_OPERAND_TYPE_FP_ROUNDING_MODE,
  SPV_OPERAND_TYPE_LINKAGE_TYPE,
  SPV_OPERAND_TYPE_ACCESS_QUALIFIER,
  SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE,
  SPV_OPERAND_TYPE_DECORATION,
  SPV_OPERAND_TYPE_BUILT_IN,
  SPV_OPERAND_TYPE_SELECTION_CONTROL,
  SPV_OPERAND_TYPE_LOOP_CONTROL,
  SPV_OPERAND_TYPE_FUNCTION_CONTROL,

  // The ID for a memory semantics value.
  SPV_OPERAND_TYPE_MEMORY_SEMANTICS,
  // The ID for an execution scope value.
  // TODO(dneto): Rev 30 changed "Execution Scope" to "Scope".  We should
  // probably do that here too.
  SPV_OPERAND_TYPE_EXECUTION_SCOPE,

  SPV_OPERAND_TYPE_GROUP_OPERATION,
  SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS,
  SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO,
  SPV_OPERAND_TYPE_CAPABILITY,

  // An optional operand represents zero or one logical operands.
  // In an instruction definition, this may only appear at the end of the
  // operand types.
  SPV_OPERAND_TYPE_OPTIONAL_ID,
  // An optional image operands mask.  A set bit in the mask may
  // imply that more arguments are required.
  SPV_OPERAND_TYPE_OPTIONAL_IMAGE,
  // An optional literal integer.
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER,
  // An optional literal string.
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING,
  // An optional memory access qualifier mask, e.g. Volatile, Aligned,
  // or a combination.
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS,
  // An optional execution mode
  SPV_OPERAND_TYPE_OPTIONAL_EXECUTION_MODE,
  // A variable operand represents zero or more logical operands.
  // In an instruction definition, this may only appear at the end of the
  // operand types.
  SPV_OPERAND_TYPE_VARIABLE_ID,
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER,
  // A sequence of zero or more pairs of (Literal integer, Id)
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID,
  // A sequence of zero or more pairs of (Id, Literal integer)
  SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER,
  // A sequence of zero or more execution modes
  SPV_OPERAND_TYPE_VARIABLE_EXECUTION_MODE,

  // An Id that is second or later in an optional tuple of operands.
  // This must be present if the first operand in the tuple is present.
  SPV_OPERAND_TYPE_ID_IN_OPTIONAL_TUPLE,
  // A Literal integer that is second or later in an optional tuple of operands.
  // This must be present if the first operand in the tuple is present.
  SPV_OPERAND_TYPE_LITERAL_INTEGER_IN_OPTIONAL_TUPLE,

  // An optional context-independent value, or CIV.  CIVs are tokens that we can
  // assemble regardless of where they occur -- literals, IDs, immediate
  // integers, etc.
  SPV_OPERAND_TYPE_OPTIONAL_CIV,
  // An optional literal number. This can expand to either a literal integer or
  // a literal floating-point number.
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_NUMBER,

  // The Instruction argument to OpExtInst. It's an unsigned 32-bit literal
  // number indicating which instruction to use from an extended instruction
  // set.
  SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER,

  // This is a sentinel value, and does not represent an operand type.
  // It should come last.
  SPV_OPERAND_TYPE_NUM_OPERAND_TYPES,

  SPV_FORCE_32_BIT_ENUM(spv_operand_type_t)
} spv_operand_type_t;

typedef enum spv_ext_inst_type_t {
  SPV_EXT_INST_TYPE_NONE,
  SPV_EXT_INST_TYPE_GLSL_STD_450,
  SPV_EXT_INST_TYPE_OPENCL_STD,

  SPV_FORCE_32_BIT_ENUM(spv_ext_inst_type_t)
} spv_ext_inst_type_t;

typedef enum spv_binary_to_text_options_t {
  SPV_BINARY_TO_TEXT_OPTION_NONE = SPV_BIT(0),
  SPV_BINARY_TO_TEXT_OPTION_PRINT = SPV_BIT(1),
  SPV_BINARY_TO_TEXT_OPTION_COLOR = SPV_BIT(2),
  SPV_FORCE_32_BIT_ENUM(spv_binary_to_text_options_t)
} spv_binary_to_text_options_t;

typedef enum spv_assembly_syntax_format_t {
  SPV_ASSEMBLY_SYNTAX_FORMAT_ASSIGNMENT,  // Assignment Assembly Format
  SPV_ASSEMBLY_SYNTAX_FORMAT_CANONICAL,   // Canonical Assembly Format
  SPV_ASSEMBLY_SYNTAX_FORMAT_DEFAULT = SPV_ASSEMBLY_SYNTAX_FORMAT_ASSIGNMENT,
} spv_assembly_syntax_format_t;

typedef enum spv_validate_options_t {
  SPV_VALIDATE_BASIC_BIT = SPV_BIT(0),
  SPV_VALIDATE_LAYOUT_BIT = SPV_BIT(1),
  SPV_VALIDATE_ID_BIT = SPV_BIT(2),
  SPV_VALIDATE_RULES_BIT = SPV_BIT(3),
  SPV_VALIDATE_ALL = SPV_VALIDATE_BASIC_BIT | SPV_VALIDATE_LAYOUT_BIT |
                     SPV_VALIDATE_ID_BIT | SPV_VALIDATE_RULES_BIT,
  SPV_FORCE_32_BIT_ENUM(spv_validation_options_t)
} spv_validate_options_t;

// Structures

typedef struct spv_header_t {
  uint32_t magic;
  uint32_t version;
  uint32_t generator;
  uint32_t bound;
  uint32_t schema;               // NOTE: Reserved
  const uint32_t *instructions;  // NOTE: Unfixed pointer to instruciton stream
} spv_header_t;

typedef struct spv_opcode_desc_t {
  const char *name;
  const Op opcode;
  const spv_capability_mask_t
      capabilities;  // Bitfield of SPV_CAPABILITY_AS_MASK(spv::Capability)
  // operandTypes[0..numTypes-1] describe logical operands for the instruction.
  // The operand types include result id and result-type id, followed by
  // the types of arguments.
  uint16_t numTypes;
  spv_operand_type_t operandTypes[16];  // TODO: Smaller/larger?
  const bool hasResult;  // Does the instruction have a result ID operand?
  const bool hasType;    // Does the instruction have a type ID operand?
  // The operand class for each logical argument.  This does *not* include
  // the result Id or type ID.  The list is terminated by SPV_OPERAND_TYPE_NONE.
  const OperandClass operandClass[16];
} spv_opcode_desc_t;

typedef struct spv_opcode_table_t {
  const uint32_t count;
  const spv_opcode_desc_t *entries;
} spv_opcode_table_t;

typedef struct spv_operand_desc_t {
  const char *name;
  const uint32_t value;
  const spv_capability_mask_t
      capabilities;  // Bitfield of SPV_CAPABILITY_AS_MASK(spv::Capability)
  const spv_operand_type_t operandTypes[16];  // TODO: Smaller/larger?
} spv_operand_desc_t;

typedef struct spv_operand_desc_group_t {
  const spv_operand_type_t type;
  const uint32_t count;
  const spv_operand_desc_t *entries;
} spv_operand_desc_group_t;

typedef struct spv_operand_table_t {
  const uint32_t count;
  const spv_operand_desc_group_t *types;
} spv_operand_table_t;

typedef struct spv_ext_inst_desc_t {
  const char *name;
  const uint32_t ext_inst;
  const spv_operand_type_t operandTypes[16];  // TODO: Smaller/larger?
} spv_ext_inst_desc_t;

typedef struct spv_ext_inst_group_t {
  const spv_ext_inst_type_t type;
  const uint32_t count;
  const spv_ext_inst_desc_t *entries;
} spv_ext_inst_group_t;

typedef struct spv_ext_inst_table_t {
  const uint32_t count;
  const spv_ext_inst_group_t *groups;
} spv_ext_inst_table_t;

typedef struct spv_binary_t {
  uint32_t *code;
  uint64_t wordCount;
} spv_binary_t;

typedef struct spv_text_t {
  const char *str;
  uint64_t length;
} spv_text_t;

typedef struct spv_position_t {
  uint64_t line;
  uint64_t column;
  uint64_t index;
} spv_position_t;

typedef struct spv_diagnostic_t {
  spv_position_t position;
  char *error;
  bool isTextSource;
} spv_diagnostic_t;

// Type Definitions

typedef const spv_opcode_desc_t *spv_opcode_desc;
typedef const spv_opcode_table_t *spv_opcode_table;
typedef const spv_operand_desc_t *spv_operand_desc;
typedef const spv_operand_table_t *spv_operand_table;
typedef const spv_ext_inst_desc_t *spv_ext_inst_desc;
typedef const spv_ext_inst_table_t *spv_ext_inst_table;
typedef spv_binary_t *spv_binary;
typedef spv_text_t *spv_text;
typedef spv_position_t *spv_position;
typedef spv_diagnostic_t *spv_diagnostic;

// Platform API

// Opcode API

/// @brief Populate the Opcode table
///
/// @param[out] pOpcodeTable table to be populated
///
/// @return result code
spv_result_t spvOpcodeTableGet(spv_opcode_table *pOpcodeTable);

/// @brief Populate the operand table
///
/// @param[in] pOperandTable table to be populated
///
/// @return result code
spv_result_t spvOperandTableGet(spv_operand_table *pOperandTable);

/// @brief Populate the extended instruction table
///
/// @param pTable table to be populated
///
/// @return result code
spv_result_t spvExtInstTableGet(spv_ext_inst_table *pTable);

// Text API

/// @brief Entry point to covert text form to binary form
///
/// @param[in] text input text
/// @param[in] length of the input text
/// @param[in] opcodeTable of specified Opcodes
/// @param[in] operandTable of specified operands
/// @param[in] extInstTable of specified extended instructions
/// @param[out] pBinary the binary module
/// @param[out] pDiagnostic contains diagnostic on failure
///
/// @return result code
spv_result_t spvTextToBinary(const char *text, const uint64_t length,
                             const spv_opcode_table opcodeTable,
                             const spv_operand_table operandTable,
                             const spv_ext_inst_table extInstTable,
                             spv_binary *pBinary, spv_diagnostic *pDiagnostic);

/// @brief Entry point to covert text form to binary form
///
/// @param[in] text input text
/// @param[in] length of the input text
/// @param[in] format the assembly syntax format of text
/// @param[in] opcodeTable of specified Opcodes
/// @param[in] operandTable of specified operands
/// @param[in] extInstTable of specified extended instructions
/// @param[out] pBinary the binary module
/// @param[out] pDiagnostic contains diagnostic on failure
///
/// @return result code
spv_result_t spvTextWithFormatToBinary(const char *text, const uint64_t length,
                                       spv_assembly_syntax_format_t format,
                                       const spv_opcode_table opcodeTable,
                                       const spv_operand_table operandTable,
                                       const spv_ext_inst_table extInstTable,
                                       spv_binary *pBinary,
                                       spv_diagnostic *pDiagnostic);

/// @brief Free an allocated text stream
///
/// This is a no-op if the text parameter is a null pointer.
///
/// @param text the text object to be destored
void spvTextDestroy(spv_text text);

// Binary API

/// @brief Entry point to convert binary to text form
///
/// @param[in] binary the input binary
/// @param[in] wordCount the number of input words
/// @param[in] options bitfield of spv_binary_to_text_options_t values
/// @param[in] opcodeTable table of specified Opcodes
/// @param[in] operandTable table of specified operands
/// @param[in] extInstTable of specified extended instructions
/// @param[out] pText the textual form
/// @param[out] pDiagnostic contains diagnostic on failure
///
/// @return result code
spv_result_t spvBinaryToText(uint32_t *binary, const uint64_t wordCount,
                             const uint32_t options,
                             const spv_opcode_table opcodeTable,
                             const spv_operand_table operandTable,
                             const spv_ext_inst_table extInstTable,
                             spv_text *pText, spv_diagnostic *pDiagnostic);

/// @brief Entry point to convert binary to text form
///
/// @param[in] binary the input binary
/// @param[in] wordCount the number of input words
/// @param[in] options bitfield of spv_binary_to_text_options_t values
/// @param[in] opcodeTable table of specified Opcodes
/// @param[in] operandTable table of specified operands
/// @param[in] extInstTable of specified extended instructions
/// @param[in] format the assembly syntax format of text
/// @param[out] pText the textual form
/// @param[out] pDiagnostic contains diagnostic on failure
///
/// @return result code
spv_result_t spvBinaryToTextWithFormat(
    uint32_t *binary, const uint64_t wordCount, const uint32_t options,
    const spv_opcode_table opcodeTable, const spv_operand_table operandTable,
    const spv_ext_inst_table extInstTable, spv_assembly_syntax_format_t format,
    spv_text *pText, spv_diagnostic *pDiagnostic);

/// @brief Free a binary stream from memory.
///
/// This is a no-op if binary is a null pointer.
///
/// @param binary stream to destroy
void spvBinaryDestroy(spv_binary binary);

// Validation API

/// @brief Validate a SPIR-V binary for correctness
///
/// @param[in] binary the input binary stream
/// @param[in] opcodeTable table of specified Opcodes
/// @param[in] operandTable table of specified operands
/// @param[in] extInstTable of specified extended instructions
/// @param[in] options bitfield of spv_validation_options_t
/// @param[out] pDiagnostic contains diagnostic on failure
///
/// @return result code
spv_result_t spvValidate(const spv_binary binary,
                         const spv_opcode_table opcodeTable,
                         const spv_operand_table operandTable,
                         const spv_ext_inst_table extInstTable,
                         const uint32_t options, spv_diagnostic *pDiagnostic);

// Diagnostic API

/// @brief Create a diagnostic object
///
/// @param position position in the text or binary stream
/// @param message error message to display, is copied
///
/// @return the diagnostic object
spv_diagnostic spvDiagnosticCreate(const spv_position position,
                                   const char *message);

/// @brief Destroy a diagnostic object
///
/// @param diagnostic object to destory
void spvDiagnosticDestroy(spv_diagnostic diagnostic);

/// @brief Print the diagnostic to stderr
///
/// @param[in] diagnostic to print
///
/// @return result code
spv_result_t spvDiagnosticPrint(const spv_diagnostic diagnostic);

#ifdef __cplusplus
}
#endif

#endif