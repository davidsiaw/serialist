require "serialist-gen/backends/c_utils/c_utils"

module SerialistGen
module Backends
module CUtils

class CBackendBase

	include SerialistGen::Backends::CUtils

	def initialize(name, ast)
		@name = name
		@ast = ast
	end

	def simple_type?(typename)
		SerialistGen::Utils::simple_type?(typename, @ast[:subsets])
	end

	def c_type(typename)
		SerialistGen::Backends::CUtils::c_type(typename, @ast[:subsets])
	end

	def type_length(typename)
		SerialistGen::Backends::CUtils::type_length(typename, @ast[:subsets])
	end

end

end
end
end