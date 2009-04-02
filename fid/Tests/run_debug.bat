@echo off
SET bindir=%4%
SET codecdir=%bindir%\codecs
SET validatordir=%bindir%\validators
SET test_it=%bindir%\fid -cod %codecdir% -vad %validatordir% -d
echo ======================================================== >>%3
echo testing %1%2 >>%3
echo -------------------------------------------------------- >>%3
%test_it% %1%2 >>%3
