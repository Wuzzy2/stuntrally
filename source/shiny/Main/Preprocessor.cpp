#include "pch.h"
#include "Preprocessor.hpp"

#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>

namespace sh
{
	std::string Preprocessor::preprocess (std::string source, const std::string& includePath, std::vector<std::string> definitions, const std::string& name)
	{
		std::stringstream returnString;

		// current file position is saved for exception handling
		boost::wave::util::file_position_type current_position;

		try
		{
			//  This token type is one of the central types used throughout the library.
			//  It is a template parameter to some of the public classes and instances
			//  of this type are returned from the iterators.
			typedef boost::wave::cpplexer::lex_token<> token_type;

			//  The template boost::wave::cpplexer::lex_iterator<> is the lexer type to
			//  to use as the token source for the preprocessing engine. It is
			//  parametrized with the token type.
			typedef boost::wave::cpplexer::lex_iterator<token_type> lex_iterator_type;

			//  This is the resulting context type. The first template parameter should
			//  match the iterator type used during construction of the context
			//  instance (see below). It is the type of the underlying input stream.
			typedef boost::wave::context<std::string::iterator, lex_iterator_type
					, boost::wave::iteration_context_policies::load_file_to_string,
					emit_custom_line_directives_hooks>
				context_type;

			//  The preprocessor iterator shouldn't be constructed directly. It is
			//  generated through a wave::context<> object. This wave:context<> object
			//  is additionally used to initialize and define different parameters of
			//  the actual preprocessing.
			//
			//  The preprocessing of the input stream is done on the fly behind the
			//  scenes during iteration over the range of context_type::iterator_type
			//  instances.
			context_type ctx (source.begin(), source.end(), name.c_str());
			ctx.add_include_path(includePath.c_str());
			for (std::vector<std::string>::iterator it = definitions.begin(); it != definitions.end(); ++it)
			{
				ctx.add_macro_definition(*it);
			}

			//  Get the preprocessor iterators and use them to generate the token
			//  sequence.
			context_type::iterator_type first = ctx.begin();
			context_type::iterator_type last = ctx.end();

			//  The input stream is preprocessed for you while iterating over the range
			//  [first, last). The dereferenced iterator returns tokens holding
			//  information about the preprocessed input stream, such as token type,
			//  token value, and position.
			while (first != last)
			{
				current_position = (*first).get_position();
				returnString << (*first).get_value();
				++first;
			}
		}
		catch (boost::wave::cpp_exception const& e)
		{
			// some preprocessing error
			std::stringstream error;
			error
				<< e.file_name() << "(" << e.line_no() << "): "
				<< e.description();
			throw std::runtime_error(error.str());
		}
		catch (std::exception const& e)
		{
			// use last recognized token to retrieve the error position
			std::stringstream error;
			error
				<< current_position.get_file()
				<< "(" << current_position.get_line() << "): "
				<< "exception caught: " << e.what();
			throw std::runtime_error(error.str());
		}
		catch (...)
		{
			// use last recognized token to retrieve the error position
			std::stringstream error;
			error
				<< current_position.get_file()
				<< "(" << current_position.get_line() << "): "
				<< "unexpected exception caught.";
			throw std::runtime_error(error.str());
		}

		return returnString.str();
	}
}
