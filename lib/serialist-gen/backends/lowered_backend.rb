
module SerialistGen
module Backends

class LoweredBackend

	def self.desc
		"Echoes the format in a lowered version of the language"
	end

	def initialize(name, ast)
		@name = name
		@ast = ast
	end

	def convert_attribute_name(name)
		case name
		when :array_size
			"ArraySize"
		when :must_contain
			"MustContain"
		else
			name
		end
	end

	def gen_lowered_expression(expr)
		if expr[:int] != nil
			"0x#{expr[:int].to_s(16)}"
		elsif expr[:call] != nil
			parms = expr[:params].map{|param| gen_expression(param)}.join(", ")
			"#{gen_expression(expr[:call])}(#{parms})"
		elsif expr[:ref] != nil
			"#{expr[:ref]}"
		elsif expr[:real] != nil
			"#{expr[:real]}"
		else
			puts "The expression:"
			p expr
			raise "Expression unknown!"
		end
	end

	def gen_extra_expression_info_comment(expr)
		if expr[:char] != nil
			"'#{expr[:char]}'"
		else
			nil
		end
	end

	def gen_expression(expr)
		extra = gen_extra_expression_info_comment(expr)

		if extra
			"#{gen_lowered_expression(expr)} /* #{extra} */"
		else
			"#{gen_lowered_expression(expr)}"
		end
		
	end

	def gen_attribute(name, value)

		parameters = ""
		if value.is_a? Array and value.length > 0
			parameters = ": " + value.map{ |x| gen_expression(x) }.join(", ")
		elsif value
			parameters = ": " + gen_expression(value)
		end

"\n	[#{convert_attribute_name(name)}#{parameters}]"
	end

	def gen_attributes(member)
		member[:attributes].map do |name, values|
			if values.is_a? Array
				values.map do |value|
					gen_attribute(name, value)
				end.join ""
			else
				gen_attribute(name, values)
			end

		end.join ""
	end

	def gen_members(members)
		members.map do |member|
			<<-MEMBEREND
#{gen_attributes(member)}
	#{member[:type]} #{member[:name]}
MEMBEREND
		end.join ""

	end

	def gen_subsets
		@ast[:subsets].map do |subset|
			<<-SUBSETEND
subset #{subset[:name]} : #{subset[:origin_type]} = #{subset[:elements].map {|x| gen_expression(x)}.join "," }

			SUBSETEND
		end.join ""
	end

	def gen_formats
		@ast[:formats].map do |format|
			<<-FORMATEND
format #{format[:name]}
{
#{gen_members(format[:members])}
}

			FORMATEND
		end.join ""
	end

	def generate
		<<-ECHOEND
#{gen_subsets}
#{gen_formats}
ECHOEND
	end
end

end
end
