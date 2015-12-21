:: 
:: Copyright (c) 2015 Jesse Nicholson
:: 
:: Permission is hereby granted, free of charge, to any person obtaining a copy
:: of this software and associated documentation files (the "Software"), to deal
:: in the Software without restriction, including without limitation the rights
:: to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
:: copies of the Software, and to permit persons to whom the Software is
:: furnished to do so, subject to the following conditions:
:: 
:: The above copyright notice and this permission notice shall be included in
:: all copies or substantial portions of the Software.
:: 
:: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
:: IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
:: FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
:: AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
:: LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
:: OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
:: THE SOFTWARE.
:: 
@echo off

:: Set BCP_LOCATION to the directory where a compiled version of bcp.exe resides.
set BCP_LOCATION=

:: Set BOOST_ROOT_LOCATION to the root directory of the boost version to extract from.
set BOOST_ROOT_LOCATION=

:: By passing these two headers to BCP, it should be enough for BCP to discover
:: and extract all boost libraries that the project includes. Not that if additional
:: files are needed, just include the headers which include the missing/required
:: boost libraries. BCP will scan all input files and extract complete dependencies
:: to the output directory.
set PARSER_HPP_LOCATION="%cd%\..\src\GQParser.hpp"
set TREEMAP_HPP_LOCATION="%cd%\..\src\GQStrRefHash.hpp"

:: Make sure the variables have been set.
if [%BCP_LOCATION%] == [] GOTO RequiresSetup
if [%BOOST_ROOT_LOCATION%] == [] GOTO RequiresSetup
if [%PARSER_HPP_LOCATION%] == [] GOTO RequiresSetup
if [%TREEMAP_HPP_LOCATION%] == [] GOTO RequiresSetup

:: Ensure that the output directory exists.
mkdir "%cd%\..\third-party"
mkdir "%cd%\..\third-party\include"

set OUTPUT_DIR="%cd%\..\third-party\include"

:: Call BCP
%BCP_LOCATION%\bcp.exe --boost=%BOOST_ROOT_LOCATION% --scan %PARSER_HPP_LOCATION% %TREEMAP_HPP_LOCATION% %OUTPUT_DIR%

:: Quit
exit /B

:: This will print out help information to the console to assist the 
:: user in correctly configuring the script in the event that the variables 
:: were not setup. 
:RequiresSetup
echo. & echo Please edit this batch file to have the following variables set correctly: & echo.
echo 	BCP_LOCATION - Set this to the directory where a compiled version of bcp.exe resides. & echo.
echo 	BOOST_ROOT_LOCATION - Set this to the root directory of the boost version to extract from. & echo.
echo 	PARSER_HPP_LOCATION - Set this to the full path of "GQParser.hpp". & echo.
echo 	TREEMAP_HPP_LOCATION - Set this to the full path of "GQTreeMap.hpp" & echo.
echo. & echo See script comments for more details.
timeout 10
exit /B