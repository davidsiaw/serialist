
module SerialistGen
module Backends

require "serialist-gen/backends/c_utils/c_utils"

class CSourceBackend

	def self.desc
		"Generates the C sources for the format"
	end

	def initialize(name, ast)
		@name = name
		@ast = ast
	end

	def convert_intrinsic_function(expr)
		parmArray = expr[:params].map{|param| generate_expression(param)}
		parms = parmArray.join(", ")
		if expr[:call][:ref] != nil
			func_name = expr[:call][:ref]
			case func_name
			when "add"
				"(#{parmArray[0]} + #{parmArray[1]})"
			when "sub"
				"(#{parmArray[0]} - #{parmArray[1]})"
			when "mul"
				"(#{parmArray[0]} * #{parmArray[1]})"
			when "div"
				"(#{parmArray[0]} / #{parmArray[1]})"

			when "and"
				"(#{parmArray[0]} & #{parmArray[1]})"
			when "or"
				"(#{parmArray[0]} | #{parmArray[1]})"
			when "xor"
				"(#{parmArray[0]} ^ #{parmArray[1]})"

			when "eq"
				"(#{parmArray[0]} == #{parmArray[1]})"
			when "neq"
				"(#{parmArray[0]} != #{parmArray[1]})"

			when "lt"
				"(#{parmArray[0]} < #{parmArray[1]})"
			when "lte"
				"(#{parmArray[0]} <= #{parmArray[1]})"
			when "gt"
				"(#{parmArray[0]} > #{parmArray[1]})"
			when "gte"
				"(#{parmArray[0]} >= #{parmArray[1]})"

			when "andb"
				"(#{parmArray[0]} && #{parmArray[1]})"
			when "orb"
				"(#{parmArray[0]} || #{parmArray[1]})"

			when "sl"
				"(#{parmArray[0]} << #{parmArray[1]})"
			when "sr"
				"(#{parmArray[0]} >> #{parmArray[1]})"

			when "if"
				"(#{parmArray[0]} ? #{parmArray[1]} : #{parmArray[2]})"
			else
				"#{func_name}(#{parms})"
			end
		else
			"#{generate_expression(expr[:call])}(#{parms})"
		end
	end

	def generate_expression(expr)
		if expr[:int] != nil
			"#{expr[:int]}"
		elsif expr[:char] != nil
			"'#{expr[:char]}'"
		elsif expr[:call] != nil
			convert_intrinsic_function(expr)
		elsif expr[:ref] != nil
			"pointer->#{expr[:ref]}"
		elsif expr[:real] != nil
			"#{expr[:real]}"
		else
			puts "The expression:"
			p expr
			raise "Expression unknown!"
		end
	end

	def generate_member_readers(format,members)
		members.map do |member|

			reader = ""
			reader_decls = ""

			if simple_type? member[:type]
				reader = <<-READER_CODE
		amt_read = fread(&member, sizeof(#{c_type(member[:type])}), 1, fp);
		if (amt_read != 1)
		{
			return SERIALIST_UNEXPECTED_END_OF_FILE;
		}
				READER_CODE

				reader_decls = <<-READER_DECLS
	size_t amt_read = 0;
				READER_DECLS
			else
				reader = <<-READER_CODE
		error_code = Read#{member[:type]}(fp, &member);
		if (error_code)
		{
			return error_code;
		}
				READER_CODE
			end

			if member[:attributes].has_key? :array_size
				<<-MEMBERREAD
SerialistError Read#{format[:name]}_#{member[:name]}(FILE* fp, #{format[:name]}* pointer)
{
	#{c_type(member[:type])} member;
#{reader_decls}
	size_t index = 0;
	size_t count = 0;
	SerialistError error_code = SERIALIST_NO_ERROR;

	if (fp == NULL)
	{
		return SERIALIST_NULL_FILE_POINTER;
	}

	error_code = Count#{format[:name]}_#{member[:name]}(pointer, &count);
	if (error_code)
	{
		return error_code;
	}

	for (index = 0; index < count; index++)
	{
#{reader}
		error_code = Set#{format[:name]}_#{member[:name]}(pointer, member, index);
		if (error_code)
		{
			return error_code;
		}
	}
	return SERIALIST_NO_ERROR;
}
				MEMBERREAD
			else
				<<-MEMBERREAD
SerialistError Read#{format[:name]}_#{member[:name]}(FILE* fp, #{format[:name]}* pointer)
{
	SerialistError error_code = SERIALIST_NO_ERROR;

	if (fp == NULL)
	{
		return SERIALIST_NULL_FILE_POINTER;
	}

	#{c_type(member[:type])} member;
#{reader_decls}
#{reader}
	error_code = Set#{format[:name]}_#{member[:name]}(pointer, member);
	if (error_code)
	{
		return error_code;
	}
	return SERIALIST_NO_ERROR;
}
				MEMBERREAD
			end

		end.join ""
	end

	def generate_member_read_calls(format,members)
		members.map do |member|
			<<-MEMBERREAD
	error_code = Read#{format[:name]}_#{member[:name]}(fp, pointer);
	if (error_code) { goto error; }
			MEMBERREAD
		end.join ""
	end

	def generate_member_write_calls(format,members)
		members.map do |member|
			<<-MEMBERREAD
	error_code = Write#{format[:name]}_#{member[:name]}(pointer, fp);
	if (error_code) { goto error; }
			MEMBERREAD
		end.join ""
	end

	def generate_member_writers(format,members)
		members.map do |member|
			if member[:attributes].has_key? :array_size
				<<-MEMBERWRITE
SerialistError Write#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, FILE* fp)
{
	#{c_type(member[:type])} item;
	size_t index = 0;
	size_t count = 0;
	SerialistError error_code = SERIALIST_NO_ERROR;

	if (fp == NULL)
	{
		return SERIALIST_NULL_FILE_POINTER;
	}

	error_code = Count#{format[:name]}_#{member[:name]}(pointer, &count);
	size_t amt_written = 0;

	for (index = 0; index < count; index++)
	{
		Get#{format[:name]}_#{member[:name]}(pointer, index, &item);
		if (error_code)
		{
			return error_code;
		}
		fwrite(&item, sizeof(#{c_type(member[:type])}), 1, fp);
		if (amt_written != sizeof(#{c_type(member[:type])}))
		{
			return SERIALIST_WRITE_FAILED;
		}
	}
	return SERIALIST_NO_ERROR;
}
				MEMBERWRITE
			else
				<<-MEMBERWRITE
SerialistError Write#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, FILE* fp)
{
	#{c_type(member[:type])} item;
	SerialistError error_code = SERIALIST_NO_ERROR;

	if (fp == NULL)
	{
		return SERIALIST_NULL_FILE_POINTER;
	}

	error_code = Get#{format[:name]}_#{member[:name]}(pointer, &item);
	size_t amt_written = 0;
	if (error_code)
	{
		return error_code;
	}
	fwrite(&item, sizeof(#{c_type(member[:type])}), 1, fp);
	if (amt_written != sizeof(#{c_type(member[:type])}))
	{
		return SERIALIST_WRITE_FAILED;
	}
	return SERIALIST_NO_ERROR;
}
				MEMBERWRITE
			end
		end.join ""
	end


	def generate_member_functions(format,members)
		members.map do |member|
			if member[:attributes].has_key? :array_size
			<<-ENDMEMBERPROTO
SerialistError Check#{format[:name]}_#{member[:name]}ArraySize(#{format[:name]}* pointer)
{
	/* Method for lazily updating the array size, so that if it is dependant on some other
	 * variable, it can be quickly checked
	 */
	size_t expected_size = 0;
	SerialistError error_code = Count#{format[:name]}_#{member[:name]}(pointer, &expected_size);
	if (error_code)
	{
		return error_code;
	}
	if (pointer->SIZEOF_#{member[:name]} != expected_size)
	{
		pointer->#{member[:name]} = realloc(pointer->#{member[:name]}, expected_size * sizeof(#{c_type(member[:type])}));
		if (pointer->#{member[:name]} == NULL)
		{
			return SERIALIST_OUT_OF_MEMORY;
		}
		if (pointer->SIZEOF_#{member[:name]} < expected_size)
		{
			/* The array size increased. We need to zero out the new memory */
			memset(&pointer->#{member[:name]}[pointer->SIZEOF_#{member[:name]}], 0, (expected_size - pointer->SIZEOF_#{member[:name]}) * sizeof(#{c_type(member[:type])}) ) ;
		}
		pointer->SIZEOF_#{member[:name]} = expected_size;
	}
	return SERIALIST_NO_ERROR;
}

SerialistError Count#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, size_t* out_count)
{
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	if (out_count == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	*out_count = #{generate_expression(member[:attributes][:array_size])};

	return SERIALIST_NO_ERROR;
}

SerialistError Set#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, #{c_type(member[:type])} value, size_t index)
{
	SerialistError error_code = Check#{format[:name]}_#{member[:name]}ArraySize(pointer);
	if (error_code)
	{
		return error_code;
	}
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	if (index >= pointer->SIZEOF_#{member[:name]})
	{
		return SERIALIST_INDEX_OUT_OF_BOUNDS;
	}

	pointer->#{member[:name]}[index] = value;
	return SERIALIST_NO_ERROR;
}

SerialistError Get#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, size_t index, #{c_type(member[:type])}* out_item)
{
	SerialistError error_code = Check#{format[:name]}_#{member[:name]}ArraySize(pointer);
	if (error_code)
	{
		return error_code;
	}
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	if (out_item == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	if (index >= pointer->SIZEOF_#{member[:name]})
	{
		return SERIALIST_INDEX_OUT_OF_BOUNDS;
	}
	*out_item = pointer->#{member[:name]}[index];
	return SERIALIST_NO_ERROR;
}
			ENDMEMBERPROTO
			else
			<<-ENDMEMBERPROTO
SerialistError Set#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, #{c_type(member[:type])} value)
{
	pointer->#{member[:name]} = value;
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	return SERIALIST_NO_ERROR;
}

SerialistError Get#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer, #{c_type(member[:type])}* out_item)
{
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	if (out_item == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	*out_item = pointer->#{member[:name]};
	return SERIALIST_NO_ERROR;
}
			ENDMEMBERPROTO
			end
		end.join ""
	end

	def generate_functions
		@ast[:formats].map do |format|
			<<-ENDSTRUCT
SerialistError Create#{format[:name]} (#{format[:name]}** out_pointer)
{
	#{format[:name]}* new_object = (#{format[:name]}*)calloc(1, sizeof(#{format[:name]}));
	if (new_object == NULL)
	{
		return SERIALIST_OUT_OF_MEMORY;
	}
	if (out_pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}
	new_object->TYPE_signature = #{format[:name].upcase}_TYPE;
	*out_pointer = new_object;
	return SERIALIST_NO_ERROR;
}

SerialistError Delete#{format[:name]} (#{format[:name]}* pointer)
{
	if (pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}

	if (!IsPointerOfType(pointer, #{format[:name].upcase}_TYPE))
	{
		return SERIALIST_POINTER_IS_WRONG_TYPE;
	}

	free(pointer);
	return SERIALIST_NO_ERROR;
}

#{generate_member_readers(format, format[:members])}
#{generate_member_writers(format, format[:members])}
SerialistError Read#{format[:name]} (FILE* fp, #{format[:name]}** out_pointer)
{
	SerialistError error_code = SERIALIST_NO_ERROR;
	#{format[:name]}* pointer;

	if (fp == NULL)
	{
		return SERIALIST_NULL_FILE_POINTER;
	}

	if (out_pointer == NULL)
	{
		return SERIALIST_NULL_POINTER;
	}

	error_code = Create#{format[:name]}(&pointer);
	if (error_code) { goto error; }
#{generate_member_read_calls(format, format[:members])}
	*out_pointer = pointer;
	return error_code;

error:
	Delete#{format[:name]}(pointer);
	return error_code;
}

SerialistError Write#{format[:name]} (#{format[:name]}* pointer, FILE* fp)
{
	SerialistError error_code = SERIALIST_NO_ERROR;

	if (fp == NULL)
	{
		return SERIALIST_NULL_FILE_POINTER;
	}

	if (!IsPointerOfType(pointer, #{format[:name].upcase}_TYPE))
	{
		return SERIALIST_POINTER_IS_WRONG_TYPE;
	}

#{generate_member_write_calls(format, format[:members])}

error:
	return error_code;
}

#{generate_member_functions(format, format[:members])}
			ENDSTRUCT
		end.join "\n"
	end

	def generate_structure_members(format)
		format[:members].map do |member|

			if member[:attributes].has_key? :array_size
				<<-ENDMEMBER
	#{c_type(member[:type])}* #{member[:name]};
	size_t SIZEOF_#{member[:name]};
				ENDMEMBER
			else
				<<-ENDMEMBER
	#{c_type(member[:type])} #{member[:name]};
				ENDMEMBER
			end

		end.join ""
	end

	def generate_structures
		@ast[:formats].map do |format|
			<<-ENDSTRUCT
struct #{format[:name]}
{
	#{@name.capitalize}Types TYPE_signature;
#{generate_structure_members(format)}
};
			ENDSTRUCT
		end.join ""
	end

	def generate

		<<-CHEADEREND
#include "#{@name.downcase}.h"

static int IsPointerOfType(void* pointer, #{@name.capitalize}Types type)
{
	if( *(#{@name.capitalize}Types*)pointer == type )
	{
		return 1;
	}
	return 0;
}

#{generate_structures}
#{generate_functions}

CHEADEREND
	end
end

end
end