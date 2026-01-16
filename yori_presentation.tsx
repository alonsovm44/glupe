import React, { useState } from 'react';
import { ChevronLeft, ChevronRight, Code, Cpu, Zap, RefreshCw, Package, Terminal } from 'lucide-react';

const YoriPresentation = () => {
  const [currentSlide, setCurrentSlide] = useState(0);

  const slides = [
    // Slide 1: Title
    {
      title: "Yori",
      subtitle: "A Local Meta-Compiler",
      content: (
        <div className="flex flex-col items-center justify-center h-full space-y-6">
          <div className="text-6xl font-bold bg-gradient-to-r from-blue-600 to-purple-600 bg-clip-text text-transparent">
            Yori
          </div>
          <div className="text-3xl text-gray-700">
            Compiling Natural Language into Self-Correcting C++ Binaries
          </div>
          <div className="text-xl text-gray-500 mt-8">
            Presented by alonsovm44
          </div>
          <div className="text-lg text-gray-400">
            {new Date().toLocaleDateString('en-US', { year: 'numeric', month: 'long', day: 'numeric' })}
          </div>
        </div>
      )
    },
    
    // Slide 2: The Problem
    {
      title: "The Problem",
      content: (
        <div className="space-y-8">
          <div className="text-2xl text-gray-700">
            Traditional programming requires constant context-switching:
          </div>
          <div className="grid grid-cols-2 gap-8">
            <div className="bg-red-50 p-6 rounded-lg border-2 border-red-200">
              <div className="text-xl font-semibold text-red-700 mb-4">Architecting</div>
              <div className="text-gray-600">
                Thinking about logic, algorithms, and system design
              </div>
            </div>
            <div className="bg-red-50 p-6 rounded-lg border-2 border-red-200">
              <div className="text-xl font-semibold text-red-700 mb-4">Implementing</div>
              <div className="text-gray-600">
                Fighting syntax, boilerplate, memory management, and compiler errors
              </div>
            </div>
          </div>
          <div className="bg-blue-50 p-6 rounded-lg border-2 border-blue-200 mt-8">
            <div className="text-xl font-semibold text-blue-700 mb-3">The Vision</div>
            <div className="text-gray-700 text-lg">
              What if we could focus purely on <span className="font-bold text-blue-600">what</span> we want to build, 
              and let the machine handle the <span className="font-bold text-blue-600">how</span>?
            </div>
          </div>
        </div>
      )
    },

    // Slide 3: What is Yori?
    {
      title: "What is Yori?",
      content: (
        <div className="space-y-6">
          <div className="text-2xl text-gray-700 mb-6">
            A meta-compiler that treats natural language as source code
          </div>
          <div className="grid grid-cols-3 gap-6">
            <div className="bg-gradient-to-br from-blue-50 to-blue-100 p-6 rounded-lg shadow-md">
              <Terminal className="w-12 h-12 text-blue-600 mb-4" />
              <div className="font-semibold text-lg mb-2">Input</div>
              <div className="text-gray-600">
                .yori files with natural language descriptions
              </div>
            </div>
            <div className="bg-gradient-to-br from-purple-50 to-purple-100 p-6 rounded-lg shadow-md">
              <Cpu className="w-12 h-12 text-purple-600 mb-4" />
              <div className="font-semibold text-lg mb-2">Process</div>
              <div className="text-gray-600">
                AI generates & self-corrects C++ code
              </div>
            </div>
            <div className="bg-gradient-to-br from-green-50 to-green-100 p-6 rounded-lg shadow-md">
              <Package className="w-12 h-12 text-green-600 mb-4" />
              <div className="font-semibold text-lg mb-2">Output</div>
              <div className="text-gray-600">
                Standalone executable binaries
              </div>
            </div>
          </div>
          <div className="bg-gray-50 p-6 rounded-lg mt-8">
            <div className="font-mono text-sm text-gray-700">
              <div className="text-blue-600 mb-2">// hello.yori</div>
              <div>PROGRAM: HelloWorld</div>
              <div className="text-green-600">// Create a program that asks for the user's name.</div>
              <div className="text-green-600">// Then print "Welcome to Yori, [Name]!"</div>
              <div className="mt-4 text-purple-600">→ yori hello.yori -local -o hello.exe</div>
              <div className="mt-2 text-green-600">→ ./hello.exe</div>
            </div>
          </div>
        </div>
      )
    },

    // Slide 4: Architecture
    {
      title: "The Three-Phase Compilation Loop",
      content: (
        <div className="space-y-6">
          <div className="flex flex-col space-y-6">
            <div className="bg-blue-50 p-6 rounded-lg border-l-4 border-blue-500">
              <div className="flex items-center mb-3">
                <div className="bg-blue-500 text-white rounded-full w-8 h-8 flex items-center justify-center font-bold mr-3">1</div>
                <div className="text-xl font-semibold text-blue-700">Draft</div>
              </div>
              <div className="text-gray-700 ml-11">
                Reads .yori file and prompts LLM (Ollama/Gemini) to generate C++ code
              </div>
            </div>

            <div className="bg-purple-50 p-6 rounded-lg border-l-4 border-purple-500">
              <div className="flex items-center mb-3">
                <div className="bg-purple-500 text-white rounded-full w-8 h-8 flex items-center justify-center font-bold mr-3">2</div>
                <div className="text-xl font-semibold text-purple-700">Verify</div>
              </div>
              <div className="text-gray-700 ml-11">
                Attempts compilation with g++ (the "Truth Filter")
              </div>
            </div>

            <div className="bg-green-50 p-6 rounded-lg border-l-4 border-green-500">
              <div className="flex items-center mb-3">
                <div className="bg-green-500 text-white rounded-full w-8 h-8 flex items-center justify-center font-bold mr-3">3</div>
                <div className="text-xl font-semibold text-green-700">Evolve</div>
              </div>
              <div className="text-gray-700 ml-11">
                If compilation fails, captures stderr, feeds it back to LLM with broken code, requests fix, and repeats until success
              </div>
            </div>
          </div>

          <div className="bg-yellow-50 p-4 rounded-lg border border-yellow-300 mt-8">
            <div className="flex items-center">
              <RefreshCw className="w-6 h-6 text-yellow-600 mr-3" />
              <div className="text-gray-700">
                <span className="font-semibold">Genetic Evolution:</span> The compiler uses g++ as an objective fitness function
              </div>
            </div>
          </div>
        </div>
      )
    },

    // Slide 5: Key Features
    {
      title: "Key Features",
      content: (
        <div className="grid grid-cols-2 gap-6">
          <div className="bg-gradient-to-br from-green-50 to-green-100 p-6 rounded-lg shadow">
            <Zap className="w-10 h-10 text-green-600 mb-3" />
            <div className="font-semibold text-lg mb-2">100% Local Operation</div>
            <div className="text-gray-600">
              Uses Ollama (qwen2.5-coder) by default. No API keys, no internet required, no usage quotas
            </div>
          </div>

          <div className="bg-gradient-to-br from-blue-50 to-blue-100 p-6 rounded-lg shadow">
            <Cpu className="w-10 h-10 text-blue-600 mb-3" />
            <div className="font-semibold text-lg mb-2">Hybrid AI Core</div>
            <div className="text-gray-600">
              Switch between local (Ollama) and cloud (Gemini) models based on your needs
            </div>
          </div>

          <div className="bg-gradient-to-br from-purple-50 to-purple-100 p-6 rounded-lg shadow">
            <RefreshCw className="w-10 h-10 text-purple-600 mb-3" />
            <div className="font-semibold text-lg mb-2">Incremental Updates (-u)</div>
            <div className="text-gray-600">
              Update existing applications without rewriting from scratch. Intelligently modifies only what's needed
            </div>
          </div>

          <div className="bg-gradient-to-br from-orange-50 to-orange-100 p-6 rounded-lg shadow">
            <Package className="w-10 h-10 text-orange-600 mb-3" />
            <div className="font-semibold text-lg mb-2">Modular Architecture</div>
            <div className="text-gray-600">
              IMPORT: tags allow splitting logic across multiple .yori files with automatic Unity Build linking
            </div>
          </div>
        </div>
      )
    },

    // Slide 6: Example - Simple
    {
      title: "Example: Hello World",
      content: (
        <div className="space-y-6">
          <div className="bg-gray-900 p-6 rounded-lg text-green-400 font-mono text-sm">
            <div className="text-blue-400 mb-3">// hello.yori</div>
            <div className="text-white">PROGRAM: HelloWorld</div>
            <div className="text-white">language: C++</div>
            <div className="mt-3 text-gray-400">// Create a program that asks for the user's name.</div>
            <div className="text-gray-400">// Then, print "Welcome to Yori, [Name]!"</div>
            <div className="text-gray-400">// Finally, print a countdown from 3 to 1.</div>
          </div>

          <div className="bg-blue-900 p-6 rounded-lg text-blue-200 font-mono text-sm">
            <div className="text-yellow-300">$ yori hello.yori -local -o hello.exe -k</div>
            <div className="mt-2">Thinking... Generating C++</div>
            <div>Compiling with g++...</div>
            <div className="text-green-400 mt-2">✓ Binary created: hello.exe</div>
          </div>

          <div className="bg-gray-900 p-6 rounded-lg text-white font-mono text-sm">
            <div className="text-yellow-300">$ ./hello.exe</div>
            <div className="mt-2">What is your name? <span className="text-blue-400">Alice</span></div>
            <div className="text-green-400">Welcome to Yori, Alice!</div>
            <div>3... 2... 1...</div>
          </div>
        </div>
      )
    },

    // Slide 7: Example - Modular
    {
      title: "Example: Modular Design",
      content: (
        <div className="space-y-6">
          <div className="grid grid-cols-2 gap-6">
            <div>
              <div className="text-sm font-semibold text-gray-600 mb-2">math.yori</div>
              <div className="bg-gray-900 p-4 rounded-lg text-green-400 font-mono text-xs">
                <div className="text-white">MODULE: MathLib</div>
                <div className="text-gray-400 mt-2">// Define a function that</div>
                <div className="text-gray-400">// calculates factorial</div>
              </div>
            </div>

            <div>
              <div className="text-sm font-semibold text-gray-600 mb-2">main.yori</div>
              <div className="bg-gray-900 p-4 rounded-lg text-green-400 font-mono text-xs">
                <div className="text-white">PROGRAM: Calculator</div>
                <div className="text-purple-400">IMPORT: math.yori</div>
                <div className="text-gray-400 mt-2">// Ask user for number</div>
                <div className="text-gray-400">// Use factorial from MathLib</div>
              </div>
            </div>
          </div>

          <div className="bg-blue-50 p-6 rounded-lg border-l-4 border-blue-500">
            <div className="text-gray-700">
              Yori automatically merges modules into a single valid compilation unit (Unity Build)
            </div>
          </div>

          <div className="bg-gray-900 p-4 rounded-lg text-white font-mono text-sm">
            <div className="text-yellow-300">$ yori main.yori -local</div>
            <div className="mt-2 text-blue-300">→ Processing imports...</div>
            <div className="text-blue-300">→ Merging MathLib module...</div>
            <div className="text-green-400 mt-2">✓ Binary created: main.exe</div>
          </div>
        </div>
      )
    },

    // Slide 8: Technical Implementation
    {
      title: "Technical Implementation",
      content: (
        <div className="space-y-6">
          <div className="grid grid-cols-2 gap-6">
            <div className="space-y-4">
              <div className="font-semibold text-xl text-gray-700 mb-4">Core Technologies</div>
              <div className="bg-blue-50 p-4 rounded-lg">
                <div className="font-semibold text-blue-700">Written in C++</div>
                <div className="text-gray-600 text-sm mt-1">Zero-dependency compiler core</div>
              </div>
              <div className="bg-purple-50 p-4 rounded-lg">
                <div className="font-semibold text-purple-700">System Tools</div>
                <div className="text-gray-600 text-sm mt-1">Uses curl and g++ from system PATH</div>
              </div>
              <div className="bg-green-50 p-4 rounded-lg">
                <div className="font-semibold text-green-700">JSON Configuration</div>
                <div className="text-gray-600 text-sm mt-1">Flexible model & API settings</div>
              </div>
            </div>

            <div className="space-y-4">
              <div className="font-semibold text-xl text-gray-700 mb-4">AI Models</div>
              <div className="bg-orange-50 p-4 rounded-lg">
                <div className="font-semibold text-orange-700">Local: Ollama</div>
                <div className="text-gray-600 text-sm mt-1">qwen2.5-coder:3b recommended</div>
                <div className="text-gray-500 text-xs mt-1">Private, offline, unlimited</div>
              </div>
              <div className="bg-red-50 p-4 rounded-lg">
                <div className="font-semibold text-red-700">Cloud: Google Gemini</div>
                <div className="text-gray-600 text-sm mt-1">gemini-2.5-flash</div>
                <div className="text-gray-500 text-xs mt-1">High-speed, high-quality</div>
              </div>
            </div>
          </div>

          <div className="bg-gray-50 p-4 rounded-lg border border-gray-300">
            <div className="font-mono text-sm text-gray-700">
              <div className="text-blue-600">config.json</div>
              <div className="mt-2">{"{"}</div>
              <div className="ml-4">"ollama_model": "qwen2.5-coder:3b",</div>
              <div className="ml-4">"gemini_api_key": "YOUR_KEY",</div>
              <div className="ml-4">"gemini_model": "gemini-2.5-flash"</div>
              <div>{"}"}</div>
            </div>
          </div>
        </div>
      )
    },

    // Slide 9: Use Cases
    {
      title: "Use Cases & Applications",
      content: (
        <div className="space-y-6">
          <div className="grid grid-cols-2 gap-6">
            <div className="bg-blue-50 p-6 rounded-lg border-l-4 border-blue-500">
              <div className="font-semibold text-lg text-blue-700 mb-3">Rapid Prototyping</div>
              <div className="text-gray-600">
                Quickly test ideas without getting bogged down in syntax
              </div>
            </div>

            <div className="bg-green-50 p-6 rounded-lg border-l-4 border-green-500">
              <div className="font-semibold text-lg text-green-700 mb-3">Education</div>
              <div className="text-gray-600">
                Learn programming concepts without syntax barriers
              </div>
            </div>

            <div className="bg-purple-50 p-6 rounded-lg border-l-4 border-purple-500">
              <div className="font-semibold text-lg text-purple-700 mb-3">System Tools</div>
              <div className="text-gray-600">
                Build custom CLI utilities from natural language specs
              </div>
            </div>

            <div className="bg-orange-50 p-6 rounded-lg border-l-4 border-orange-500">
              <div className="font-semibold text-lg text-orange-700 mb-3">Iterative Development</div>
              <div className="text-gray-600">
                Use update mode (-u) to evolve software incrementally
              </div>
            </div>
          </div>

          <div className="bg-yellow-50 p-6 rounded-lg border-2 border-yellow-300 mt-6">
            <div className="font-semibold text-lg text-yellow-800 mb-2">Design Philosophy</div>
            <div className="text-gray-700">
              "You become the <span className="font-bold">Architect</span>. You provide the <span className="font-bold">What</span>, 
              and the Yori Engine determines the <span className="font-bold">How</span>."
            </div>
          </div>
        </div>
      )
    },

    // Slide 10: Challenges & Future Work
    {
      title: "Challenges & Future Directions",
      content: (
        <div className="space-y-6">
          <div className="space-y-4">
            <div className="text-xl font-semibold text-gray-700">Current Challenges</div>
            <div className="grid grid-cols-2 gap-4">
              <div className="bg-red-50 p-4 rounded-lg border-l-4 border-red-400">
                <div className="font-semibold text-red-700 mb-2">Compilation Time</div>
                <div className="text-gray-600 text-sm">Local mode depends on hardware; cloud recommended for slower machines</div>
              </div>
              <div className="bg-red-50 p-4 rounded-lg border-l-4 border-red-400">
                <div className="font-semibold text-red-700 mb-2">Model Limitations</div>
                <div className="text-gray-600 text-sm">Quality varies between local and cloud models</div>
              </div>
            </div>
          </div>

          <div className="space-y-4 mt-8">
            <div className="text-xl font-semibold text-gray-700">Future Roadmap</div>
            <div className="grid grid-cols-2 gap-4">
              <div className="bg-blue-50 p-4 rounded-lg border-l-4 border-blue-400">
                <div className="font-semibold text-blue-700 mb-2">Multi-Language Support</div>
                <div className="text-gray-600 text-sm">Extend beyond C++ to Python, Rust, Go, etc.</div>
              </div>
              <div className="bg-blue-50 p-4 rounded-lg border-l-4 border-blue-400">
                <div className="font-semibold text-blue-700 mb-2">Enhanced Debugging</div>
                <div className="text-gray-600 text-sm">Better error messages and debugging tools</div>
              </div>
              <div className="bg-blue-50 p-4 rounded-lg border-l-4 border-blue-400">
                <div className="font-semibold text-blue-700 mb-2">IDE Integration</div>
                <div className="text-gray-600 text-sm">Plugins for VS Code, Vim, etc.</div>
              </div>
              <div className="bg-blue-50 p-4 rounded-lg border-l-4 border-blue-400">
                <div className="font-semibold text-blue-700 mb-2">Optimization</div>
                <div className="text-gray-600 text-sm">Faster compilation, smarter caching</div>
              </div>
            </div>
          </div>
        </div>
      )
    },

    // Slide 11: Live Demo
    {
      title: "Live Demo",
      content: (
        <div className="flex flex-col items-center justify-center h-full space-y-8">
          <Code className="w-24 h-24 text-blue-600" />
          <div className="text-4xl font-bold text-gray-700">
            Live Demonstration
          </div>
          <div className="text-xl text-gray-500 text-center max-w-2xl">
            Let's compile a program from natural language in real-time
          </div>
          <div className="bg-yellow-50 p-6 rounded-lg border-2 border-yellow-300 max-w-xl">
            <div className="text-gray-700 text-center">
              <div className="font-semibold mb-2">Suggested Demo:</div>
              <div>Create a simple calculator or file organizer using Yori, showing the full compilation cycle</div>
            </div>
          </div>
        </div>
      )
    },

    // Slide 12: Conclusion
    {
      title: "Conclusion",
      content: (
        <div className="flex flex-col justify-center h-full space-y-8">
          <div className="text-3xl font-bold text-gray-700 text-center">
            Bridging Intent and Implementation
          </div>
          
          <div className="grid grid-cols-3 gap-6">
            <div className="bg-blue-50 p-6 rounded-lg text-center">
              <div className="text-4xl font-bold text-blue-600 mb-2">100%</div>
              <div className="text-gray-600">Local & Private</div>
            </div>
            <div className="bg-purple-50 p-6 rounded-lg text-center">
              <div className="text-4xl font-bold text-purple-600 mb-2">Self</div>
              <div className="text-gray-600">Correcting</div>
            </div>
            <div className="bg-green-50 p-6 rounded-lg text-center">
              <div className="text-4xl font-bold text-green-600 mb-2">Open</div>
              <div className="text-gray-600">Source</div>
            </div>
          </div>

          <div className="bg-gradient-to-r from-blue-50 to-purple-50 p-8 rounded-lg border-2 border-blue-200">
            <div className="text-xl text-gray-700 text-center mb-4">
              "Making programming accessible by transforming developers into architects"
            </div>
          </div>

          <div className="text-center space-y-3">
            <div className="text-gray-600">GitHub: github.com/alonsovm44/yori</div>
            <div className="text-2xl font-semibold text-gray-700">Questions?</div>
          </div>
        </div>
      )
    }
  ];

  const nextSlide = () => {
    setCurrentSlide((prev) => (prev + 1) % slides.length);
  };

  const prevSlide = () => {
    setCurrentSlide((prev) => (prev - 1 + slides.length) % slides.length);
  };

  const goToSlide = (index) => {
    setCurrentSlide(index);
  };

  return (
    <div className="w-full h-screen bg-gray-100 flex flex-col">
      {/* Main slide area */}
      <div className="flex-1 bg-white m-8 rounded-lg shadow-2xl p-12 overflow-auto">
        {slides[currentSlide].title && (
          <h1 className="text-4xl font-bold text-gray-800 mb-8 border-b-4 border-blue-500 pb-4">
            {slides[currentSlide].title}
          </h1>
        )}
        {slides[currentSlide].subtitle && (
          <h2 className="text-2xl text-gray-600 mb-8">
            {slides[currentSlide].subtitle}
          </h2>
        )}
        <div className="mt-6">
          {slides[currentSlide].content}
        </div>
      </div>

      {/* Controls */}
      <div className="bg-gray-800 text-white p-4 flex items-center justify-between">
        <button
          onClick={prevSlide}
          className="flex items-center space-x-2 px-4 py-2 bg-gray-700 hover:bg-gray-600 rounded-lg transition-colors"
        >
          <ChevronLeft className="w-5 h-5" />
          <span>Previous</span>
        </button>

        <div className="flex items-center space-x-2">
          {slides.map((_, index) => (
            <button
              key={index}
              onClick={() => goToSlide(index)}
              className={`w-3 h-3 rounded-full transition-all ${
                index === currentSlide
                  ? 'bg-blue-500 w-8'
                  : 'bg-gray-600 hover:bg-gray-500'
              }`}
            />
          ))}
        </div>

        <div className="flex items-center space-x-4">
          <span className="text-gray-400">
            {currentSlide + 1} / {slides.length}
          </span>
          <button
            onClick={nextSlide}
            className="flex items-center space-x-2 px-4 py-2 bg-blue-600 hover:bg-blue-500 rounded-lg transition-colors"
          >
            <span>Next</span>
            <ChevronRight className="w-5 h-5" />
          </button>
        </div>
      </div>
    </div>
  );
};

export default YoriPresentation;