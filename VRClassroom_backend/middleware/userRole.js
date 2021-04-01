const Roles = require('koa-roles');

const user = new Roles({
    async failureHandler(ctx, action) {
        ctx.status = 403;
        let t = ctx.accepts('json', 'html');
        if (t === 'json') {
            ctx.body = {
                message: 'Access Denied - You don\'t have permission to: ' + action
            };
        } else if (t === 'html') {
            ctx.render('access-denied', {action: action});
        } else {
            ctx.body = 'Access Denied - You don\'t have permission to: ' + action;
        }
    }
});

// General User
user.use(async (ctx, action) => {
    return ctx.user;
});

// Access to who is attending a meeting
// user.use(async (ctx, action) => {
//     return (ctx.user.role_name === 'Host'
//         || ctx.user.role_name === 'Admin')
//         && action === 'access attendees';
// });

// Admin only
user.use('access admin', ctx => {
    if (ctx.user.role === 'Admin') {
        return true;
    }
});

module.exports = user;