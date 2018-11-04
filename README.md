# C++ TESTTASK
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1. Напишите программу, которая отсортирует числа формата double<br>
хранящихся в текстовом файле размером 1Гб (одно число в одной строке).<br>
<br>
Пример<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• 8.33891e+307<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• 1.26192e+308<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• <br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• 8.19572e+307<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• ...<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• 0<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1.64584e+304<br>
<br>
Программа должна использовать не более 100Мб оперативной памяти, и<br>
работать не дольше 25-30 минут (на 2Гц современном одноядерном процессоре).<br>
Обязательные параметры: <имя файла не отсортированного> <имя файла отсортированного><br>
Также должен быть написан генератор не отсортированного 1Гб файла с числами формата double<br>
<br>
<br>
2. Написать собственную реализацию shared_mutex поддерживающий рекурсию. Не используя C14.<br>
Использовать платформонезависимый код – средства stl и boost (но не использовать  shared_mutex).<br>
<br>
<br>
3. Написать алгоритм поиска текста по маске с wildcards (можно просто функцию куда передается два параметра - строка поиска и строка с маской). Wildcards содержит символы * и ?<br>
<br>
4. Представим что есть интерфейс к БД: <br>
<br>
struct i_db<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;bool begin_transaction();<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;bool commit_transaction();<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;bool abort_transaction();<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string get(const std::string& key);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string set(const std::string& key, const std::string& data);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string delete(const std::string& key);<br>
}<br>
<br>
Написать реализацию кеша для БД, обратить внимание на многопоточность и на транзакционную модель работы с БД.<br>
<br>
На выполнение заданий сутки.<br>
(наверное имеется в виду 8h)<br>
так как сутки 8*3=24 три дня, подразумевается 8h<br>
