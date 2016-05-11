
require "serialist-gen/backends/c_utils/c_backend_base"

module SerialistGen
module Backends

require 'tsort'

class CJSONSourceBackend < CUtils::CBackendBase

	def self.desc
		"Generates C++ code to output file data in JSON format"
	end

	def generate_member_readers(format)
		members = format[:members].map do |member|

			if member[:attributes].has_key? :array_size

				if simple_type?(member[:type])
					<<-ENDMEMBERPROTO
picojson::value JSONRead#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer)
{
	#{c_type(member[:type])} item;

	picojson::array arr;

	size_t count;
	Count#{format[:name]}_#{member[:name]}(pointer, &count);

	for(size_t i=0; i<count; i++)
	{
		SerialistError err = Get#{format[:name]}_#{member[:name]}(pointer, i, &item);
		arr.push_back(picojson::value((double)item));
	}

	return picojson::value(arr);
}
					ENDMEMBERPROTO
				else
					<<-ENDMEMBERPROTO
picojson::value JSONRead#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer)
{
	#{c_type(member[:type])} item;

	picojson::array arr;

	size_t count;
	Count#{format[:name]}_#{member[:name]}(pointer, &count);

	for(size_t i=0; i<count; i++)
	{
		SerialistError err = Get#{format[:name]}_#{member[:name]}(pointer, i, &item);

		picojson::object obj;
		obj[L"#{member[:type]}"] = JSONRead#{member[:type]}(item);
		arr.push_back(picojson::value(obj));
	}
	return picojson::value(arr);
}
					ENDMEMBERPROTO
				end

			else
				if simple_type?(member[:type])
					<<-ENDMEMBERPROTO
picojson::value JSONRead#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer)
{
	#{c_type(member[:type])} item;
	SerialistError err = Get#{format[:name]}_#{member[:name]}(pointer, &item);

	return picojson::value((double)item);
}
					ENDMEMBERPROTO
				else
					<<-ENDMEMBERPROTO
picojson::value JSONRead#{format[:name]}_#{member[:name]}(#{format[:name]}* pointer)
{
	#{c_type(member[:type])} item;
	SerialistError err = Get#{format[:name]}_#{member[:name]}(pointer, &item);

	picojson::object obj;
	obj[L"#{member[:type]}"] = JSONRead#{member[:type]}(item);

	return picojson::value(obj);
}
					ENDMEMBERPROTO
				end

			end

		end.join("")
	end

	class TsortableHash < Hash
		include TSort

		alias tsort_each_node each_key
		def tsort_each_child(node, &block)
			fetch(node).each(&block)
		end
	end

	def generate

		dependency_hash = TsortableHash.new
		format_hash = {}

		@ast[:formats].each do |format|
			used_types = []

			format[:members].each do |member|
				used_types << member[:type] if !simple_type?(member[:type]) and @ast[:subsets][member[:type]] == nil
			end

			dependency_hash[format[:name]] = used_types
			format_hash[format[:name]] = format
		end

		order = dependency_hash.tsort

		formats = order.map {|format_name| format_hash[format_name]}.map do |format|

			members = format[:members].map do |member|
				<<-ENDMEMBERPROTO
	obj[L"#{member[:name]}"] = JSONRead#{format[:name]}_#{member[:name]}(pointer);
				ENDMEMBERPROTO
		end.join ""

			<<-ENDSTRUCT
#{generate_member_readers(format)}

// #{format[:name]}
picojson::value JSONRead#{format[:name]}(#{format[:name]}* pointer)
{

	picojson::object obj;

#{members}

	return picojson::value(obj);
}
			ENDSTRUCT
		end.join "\n"

		<<-ECHOEND
		
#include "picojson.hpp"
#include "#{@name.downcase}.h"

#{formats}

		ECHOEND
	end
end

end
end