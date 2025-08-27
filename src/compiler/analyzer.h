source;
} Analyzer;

// Initialize the analyzer
void analyzer_init(Analyzer* analyzer, const char* source_file, const char* source);

// Free analyzer resources
void analyzer_free(Analyzer* analyzer);

// Analyze a list of statements (returns true if no errors)
bool analyzer_analyze(Analyzer* analyzer, Stmt** statements, int count);

// Get the number of errors
int analyzer_error_count(Analyzer* analyzer);

// Get an error by index
Error* analyzer_get_error(Analyzer* analyzer, int index);

// Print all errors
void analyzer_print_errors(Analyzer* analyzer, FILE* out);

// Declare a global variable (for native functions, etc.)
void analyzer_declare_global(Analyzer* analyzer, const char* name);

#endif // PH_ANALYZER_H
