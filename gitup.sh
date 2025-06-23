#!/bin/bash

echo "変更内容を入力してください："
read commit_message

git add .
git commit -am "$commit_message"
git push