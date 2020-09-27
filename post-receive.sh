TARGET="/home/pi/sandpit/adxl-pi"
GIT_DIR="/home/pi/repo/adxl-pi.git"
BRANCH="build"

ref=$1
if [ "$ref" = "refs/heads/$BRANCH" ];
then
	echo "Ref $ref received. Deploying ${BRANCH} branch to production..."
	mkdir -p $TARGET
	git --work-tree=$TARGET --git-dir=$GIT_DIR checkout -f $BRANCH
	cd $TARGET
	make
else
	echo "Ref $ref received. Doing nothing: only the ${BRANCH} branch may be deployed on this server."
fi
