---
layout: default
title: Release notes
---
<div class="page-content wc-container">
	<h1>Release Notes</h1>	
	{% for post in site.posts %}
		{% if post.categories contains 'release notes' %}
			{% capture currentyear %}{{post.date | date: "%Y"}}{% endcapture %}
			{% if currentyear != year %}
				{% unless forloop.first %}</ul>{% endunless %}
				<h5>{{ currentyear }}</h5>
				<ul class="posts">
				{% capture year %}{{currentyear}}{% endcapture %} 
			{% endif %}
			<li><a href="{{ post.url | prepend: site.baseurl }}">{{ post.title }}</a></li>
		{% endif %}
	{% endfor %}
</div>
