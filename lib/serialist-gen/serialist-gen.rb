require "serialist-gen/version"
require "serialist-gen/ast_converter"

Dir["#{File.dirname(__FILE__)}/backends/*.rb"].each do |backend|
	# Require all backends in the backend directory. If you want to hide files,
	# put them in a subdir
	require backend
end

module SerialistGen

	def self.available_backends
		Backends.constants.select {|c| Class === Backends.const_get(c)}
	end

	def self.generate(name, ast, backend)
		backendClass = Backends.const_get( :"#{backend}Backend" )
		backend = backendClass.new(name, ast)
		backend.generate
	end
end
