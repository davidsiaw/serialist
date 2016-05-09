
module SerialistGen
module Backends

require "serialist-gen/backends/c_utils/c_utils"

class CHeaderBackend

	def self.desc
		"Generates a C header to the sources for the format"
	end

	def initialize(name, ast)
		@name = name
		@ast = ast
	end

	def generate_member_function_prototypes(format,members)
		members.map do |member|
			if member[:attributes].has_key? :array_size
			<<-ENDMEMBERPROTO
/* #{format[:name]}::#{member[:name]} */
SerialistError Count#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, size_t* out_count);
SerialistError Set#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, #{c_type(member[:type])} value, size_t index);
SerialistError Get#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, size_t index, #{c_type(member[:type])}* out_item);
			ENDMEMBERPROTO
			else
			<<-ENDMEMBERPROTO
/* #{format[:name]}::#{member[:name]} */
SerialistError Set#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, #{c_type(member[:type])} value);
SerialistError Get#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, #{c_type(member[:type])}* out_item);
			ENDMEMBERPROTO
			end
		end.join ""
	end

	def generate_function_prototypes
		@ast[:formats].map do |format|
			<<-ENDSTRUCT
SerialistError Create#{format[:name]} (#{format[:name]}** out_pointer);
SerialistError Delete#{format[:name]} (#{format[:name]}* pointer);
SerialistError Read#{format[:name]} (FILE* fp, #{format[:name]}** out_pointer);
SerialistError Write#{format[:name]} (#{format[:name]}* pointer, FILE* fp);
#{generate_member_function_prototypes(format,format[:members])}
			ENDSTRUCT
		end.join "\n"
	end

	def generate_type_enums
		@ast[:formats].map do |format|
			"\t#{format[:name].upcase}_TYPE"
		end.join ",\n"
	end

	def generate_structure_prototypes
		@ast[:formats].map do |format|
			<<-ENDSTRUCT
typedef struct #{format[:name]} #{format[:name]};
			ENDSTRUCT
		end.join "\n"
	end

	def generate

		header_name = @name.upcase.gsub(/[^\w]+/, "_")

		error_list = ""

		for i in 0...error_types.length
			comma = "," if i < error_types.length-1
			line = <<-LINE
	#{error_types[i]} = #{i}#{comma}
			LINE
			error_list << line
		end

		<<-CHEADEREND
#ifndef #{header_name}_H
#define #{header_name}_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERIALIST_LITTLE_ENDIAN 0
#define SERIALIST_BIG_ENDIAN 1

typedef enum
{
#{error_list}
} SerialistError;

typedef enum
{
#{generate_type_enums}
} #{@name.capitalize}Types;

const char* SerialistErrorString(SerialistError err);
const wchar_t* SerialistErrorWString(SerialistError err);

#{generate_structure_prototypes}
#{generate_function_prototypes}

#ifdef __cplusplus
}
#endif

#endif // #{header_name}_H
CHEADEREND
	end
end

end
end