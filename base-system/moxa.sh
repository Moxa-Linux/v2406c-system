TMOUT=300
export TMOUT

echo "You are using Moxa embedded computer."
if [ "${USER}" = "moxa" ]; then
	echo "Please change the default password in consideration of higher security level or disable the default user, ${USER}."
fi
