
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
		else
			name
		end
	end

	def gen_expression(expr)
		if expr[:int] != nil
			"#{expr[:int]}"
		elsif expr[:char] != nil
			"'#{expr[:char]}'"
		elsif expr[:call] != nil
			parms = expr[:params].map{|param| gen_expression(param)}.join(", ")
			"#{gen_expression(expr[:call])}(#{parms})"
		elsif expr[:ref] != nil
			"#{expr[:ref]}"
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
#{gen_formats}
ECHOEND
	end
end

end
end
