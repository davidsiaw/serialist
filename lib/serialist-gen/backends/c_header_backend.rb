
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

	def generate_structure_prototypes
		@ast[:formats].map do |format|
			<<-ENDSTRUCT
typedef struct #{format[:name]} #{format[:name]};
			ENDSTRUCT
		end.join "\n"
	end

	def generate
		<<-CHEADEREND
#ifndef #{@name.upcase}_H
#define #{@name.upcase}_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
	SERIALIST_NO_ERROR				 = 0,
	SERIALIST_OUT_OF_MEMORY			 = 1,
	SERIALIST_INDEX_OUT_OF_BOUNDS	 = 2,
	SERIALIST_UNEXPECTED_END_OF_FILE = 3,
	SERIALIST_NULL_POINTER			 = 4,
	SERIALIST_WRITE_FAILED			 = 5
} SerialistError;

#{generate_structure_prototypes}
#{generate_function_prototypes}

#endif // #{@name.upcase}_H
CHEADEREND
	end
end

end
end