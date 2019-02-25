#!/usr/bin/ruby -w
# -*- coding: UTF-8 -*-
require 'json'

config = {}
print "请输入 运行main_daemon的（脚本）程序位置（绝对路径）："
config["run_daemon_script"]=gets.chop
print "请输入 重启main_daemon的（脚本）程序位置（绝对路径）："
config["restart_script"]=gets.chop
print "请输入 main_daemon的配置文件（.xml）位置（绝对路径）："
config["config_xml"]=gets.chop
print "请输入 xml配置文件（#{config["config_xml"]}）中的插件名称(<module>中name的值) ："
config["kv_plugin_name"]=gets.chop
print "请输入 修改.xml一些选项的小工具run_modify_load程序位置（绝对路径）："
config["run_modify_load"]=gets.chop
print "请输入 扫描绝对路径，本路径下至少有两个文件 ："
config["scan_path"]=gets.chop
print "请输入 扫描绝对路径，本路径下有且只有一个病毒文件 ："
config["special_scan_path"]=gets.chop
print "请输入 扫描选项，可用扫描选项有Unzip, Unpack, StopOnOne, ProgramOnly, OriginalMd5,UseFigner, UseCloud, Backup, ForceUnzip ："
config["scan_option"]=gets.chop
print "请输入 扫描记录类型（数字），0-仅显示错误（标准输出），1-全部显示（标准输出），2-仅显示错误（文件），3-全部显示（文件） ："
config["log_method"]=gets.chop

chapter1={}
chapter1[1]="修改配置文件不挂载杀毒引擎插件；重启Daemon；调用av_sdk_init，返回NULL。"
chapter1[2]="修改配置文件挂载杀毒引擎插件；重启Daemon；调用av_sdk_init获得指针；以此指针调用av_sdk_uninit，返回1。"
chapter1[3]="用空指针调用av_sdk_uninit，程序崩溃。"
config["chapter_1"]=chapter1
config["chapter_1_count"]=chapter1.count.to_s
chapter2={}
chapter2[1]="以ptr_sdk为空指针调用av_scan_target，程序崩溃。"
chapter2[2]="以uint32_option为0而其它参数正常调用av_scan_target，返回0。"
chapter2[3]="以ptr_path为NULL而其它参数正常调用av_scan_target，返回0。"
chapter2[4]="以ptr_notify为NULL而其它参数正常调用av_scan_target，返回1。"
chapter2[5]="以ptr_param为NULL而其它参数正常调用av_scan_target，返回1。"
chapter2[6]="以所有参数正常时调用av_scan_target，返回1。"
chapter2[7]="以所有参数正常时调用av_scan_target，返回1；触发回调函数，回调函数的ptr_param参数和av_scan_target传入的相等。"
chapter2[8]="以所有参数正常时调用av_scan_target，返回1；触发回调函数，回调函数的file参数不为NULL。"
chapter2[9]="以所有参数正常时调用av_scan_target，返回1；触发回调函数，回调函数的flag参数不为0时，description参数不为NULL。"
config["chapter_2"]=chapter2
config["chapter_2_count"]=chapter2.count.to_s
chapter3={}
chapter3[1]="以空指针调用av_sdk_stop，程序崩溃。"
chapter3[2]="以空指针调用av_sdk_pause，程序崩溃。"
chapter3[3]="以空指针调用av_sdk_resume，程序崩溃。"
chapter3[4]="重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_stop，返回0。"
chapter3[5]="重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_pause，返回0。"
chapter3[6]="重启Daemon；在未调用av_scan_target时，以正常参数调用av_sdk_resume，返回0。"
chapter3[7]="以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_stop，返回0。"
chapter3[8]="以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_pause，返回0。"
chapter3[9]="以正常参数调用av_scan_target；在确认扫描结束之后，以正常参数调用av_sdk_resume，返回0。"
chapter3[10]="以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_stop，返回1。且后续不再有回调函数触发。"
chapter3[11]="以正常参数调用av_scan_target扫描含有两个以上文件的目录，在第一次触发回调函数时，在回调函数中以正常参数调用av_sdk_pause，返回1；第一次回调结束后，后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发，且后续回调函数触发的总个数等于文件个数减1。"
chapter3[12]="以正常参数调用av_scan_target；在确认扫描结束之前，在回调函数之外，以正常参数调用av_sdk_stop，返回1；后续不再有回调函数触发。"
chapter3[13]="以正常参数调用av_scan_target；在确认扫描结束之后，在回调函数之外，以正常参数调用av_sdk_pause，返回1；后续不再有回调函数触发；以正常参数调用av_sdk_resume，后续有回调函数触发，且总回调函数个数等于文件个数。"
config["chapter_3"]=chapter3
config["chapter_3_count"]=chapter3.count.to_s
chapter4={}
chapter4[1]="以ptr_sdk为空指针调用av_list_file_in_quarantine，程序崩溃。"
chapter4[2]="以ptr_sdk为空指针调用av_restore_file，程序崩溃。"
chapter4[3]="以ptr_notify为空指针其它参数正常调用av_list_file_in_quarantine，返回0。"
chapter4[4]="以ptr_name为空指针其它参数正常调用av_restore_file，返回0。"
chapter4[5]="以所有参数正常时调用av_list_file_in_quarantine，返回1。"
chapter4[6]="以所有参数正常时调用av_restore_file，返回1。"
chapter4[7]="调用av_scan_target扫描一个病毒文件，保证有文件被隔离；以所有参数正常时调用av_list_file_in_quarantine，返回1；触发ListNotify回调函数时，函数参数file_old、file_new、sha不为空，size参数不为0。"
chapter4[8]="调用av_scan_target扫描一个病毒文件，保证有文件被隔离；以所有参数正常时调用av_list_file_in_quarantine，返回1；触发ListNotify回调函数时，如果file_old指向的文件不存在，以正常参数调用av_restore_file，返回1。确认file_old指向的文件存在。"
chapter4[9]="调用av_scan_target扫描一个病毒文件，保证有文件被隔离；以所有参数正常时调用av_list_file_in_quarantine，返回1；触发ListNotify回调函数时，如果file_old指向的文件不存在，获取file_new；以获取的file_new为参数，其它参数正常调用av_restore_file，返回1。确认file_old指向的文件存在。"
chapter4[10]="调用av_scan_target扫描一个病毒文件，保证有文件被隔离；以所有参数正常时调用av_list_file_in_quarantine，返回1；触发ListNotify回调函数时，如果file_old指向的文件存在，以file_new为参数，其它参数正常调用av_restore_file，返回1。确认file_old指向的文件已经被覆盖（修改日期）。"
config["chapter_4"]=chapter4
config["chapter_4_count"]=chapter4.count.to_s

config["chapter_count"]=config.values.select{|x|x.is_a?(Hash)}.count.to_s


puts config.to_json.inspect
print"以上json正确?y/n>>>"
temp = gets.chop.to_s
if temp == "y" or temp =="Y"
		print "正在生成json文件"
		File.write("scan_config.json", config.to_json)
		print " 完成"
else
	puts "请重新运行此程序"
end


